package org.csu.restaurant.restaurantserver.service;

import org.csu.restaurant.restaurantserver.dto.OrderDTO;
import org.csu.restaurant.restaurantserver.dto.OrderTaskDTO;

//限定注入指定名称的线程池 Bean
import org.springframework.beans.factory.annotation.Qualifier;
//线程池满、拒绝任务时抛出的异常
import org.springframework.core.task.TaskRejectedException;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;
import org.csu.restaurant.restaurantserver.entity.DiningTable;
import org.csu.restaurant.restaurantserver.entity.Order;
import org.csu.restaurant.restaurantserver.mapper.TableMapper;

//桌台 Mybatis 数据库操作接口
import java.util.UUID;
//生成全局唯一任务 ID
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Executor;

@Service
public class OrderQueueService {
    //自定义订单专用线程池，所有下单任务丢这里异步执行
    private final Executor orderTaskExecutor;
    private final OrderService orderService;
    //操作桌台表：查询桌台、修改桌台状态
    private final TableMapper tableMapper;
    //任务状态保存位置，内存存储所有异步下单任务
    private final ConcurrentHashMap<String, OrderTaskDTO> tasks = new ConcurrentHashMap<>();
   //桌台锁
    private final ConcurrentHashMap<Integer, Object> tableLocks = new ConcurrentHashMap<>();
    //记录每张桌台有多少个异步订单尚未处理完,配合桌台锁使用的
    private final ConcurrentHashMap<Integer, Integer> activeByTable = new ConcurrentHashMap<>();

    public OrderQueueService(@Qualifier("orderTaskExecutor") Executor orderTaskExecutor,
                             OrderService orderService,
                             TableMapper tableMapper) {
        this.orderTaskExecutor = orderTaskExecutor;
        this.orderService = orderService;
        this.tableMapper = tableMapper;
    }

    //把生成任务并交给线程池
    public OrderTaskDTO enqueue(OrderDTO orderDTO) {
    //调用下方 validate 方法，校验前端传入的订单参数合法性，不合法直接抛异常。
        validate(orderDTO);
    //订单进入异步队列,使用UUID生成唯一taskId；
        String taskId = UUID.randomUUID().toString();
    //创建状态为QUEUED的任务；将任务保存在内存中，存入 tasks 全局 Map。
        OrderTaskDTO queued = new OrderTaskDTO(taskId, "QUEUED", "订单已进入处理队列");
        tasks.put(taskId, queued);
      //  该桌台待处理订单计数 + 1。
        activeByTable.merge(orderDTO.getTableId(), 1, Integer::sum);

        try {
            orderTaskExecutor.execute(() -> process(taskId, orderDTO));
        } catch (TaskRejectedException ex) {
            tasks.remove(taskId);  //内存中删除这条失败任务记录。
            //桌台订单计数减 1；如果减到 0 直接移除这条桌台记录。
            activeByTable.computeIfPresent(orderDTO.getTableId(), (id, count) -> count <= 1 ? null : count - 1);
            throw new IllegalStateException("订单队列繁忙，请稍后重试");
        }
        return queued;
    }

    // 查询任务进度，根据 taskId 读取当前订单状态：排队 / 处理中 / 成功 / 失败。
    public OrderTaskDTO getTask(String taskId) {
        OrderTaskDTO task = tasks.get(taskId);
        if (task == null) {
            throw new IllegalArgumentException("订单任务不存在");
        }
        return task;
    }

    //后台处理订单
    private void process(String taskId, OrderDTO orderDTO) {
        //更新任务状态为处理中，同步到内存 Map。
        tasks.put(taskId, new OrderTaskDTO(taskId, "PROCESSING", "正在处理订单"));
        //异步下单时使用桌台锁
        //获得桌台的专用锁
        Object tableLock =
                tableLocks.computeIfAbsent(orderDTO.getTableId(), id -> new Object());
        //获得桌台的专用锁、只有获得当前桌台锁的线程，才能进入大括号执行订单处理。
        synchronized (tableLock) {
            try {
                //真正执行库存检查、创建订单、保存订单明细等业务的方法
                orderService.submit(orderDTO);
                //当前桌台待处理订单计数 - 1；减到 0 就移除键值对。
                activeByTable.computeIfPresent(orderDTO.getTableId(),
                        (id, count) -> count <= 1 ? null : count - 1);
                //任务状态更新为成功。
                tasks.put(taskId, new OrderTaskDTO(taskId, "SUCCESS", "下单成功"));
            } catch (Exception ex) {
                activeByTable.computeIfPresent(orderDTO.getTableId(),
                        (id, count) -> count <= 1 ? null : count - 1);
                String message = ex.getMessage() == null ? "订单处理失败" : ex.getMessage();
                tasks.put(taskId, new OrderTaskDTO(taskId, "FAILED", message));
            }
        }
    }

    //桌台结账方法（加事务）
    @Transactional
    public Order checkout(Integer tableId) {
        if(tableId == null) {
            throw new IllegalArgumentException("桌台不能为空");
        }
        Object tableLock = tableLocks.computeIfAbsent(tableId, id -> new Object());
        synchronized (tableLock) {
            //结账时也使用同一把桌台锁
            //判断该桌是否存在未跑完的异步下单任务，有则禁止结账，避免订单还没入库就结账。
            if(activeByTable.getOrDefault(tableId, 0) > 0) {
                throw new IllegalStateException("还有订单正在后台处理，请稍后再结账");
            }
            DiningTable table = tableMapper.findById(tableId);
            if(table == null) {
                throw new IllegalArgumentException("桌台不存在");
            }
            Order order = orderService.findUnpaid(tableId);
            //无订单不能结账。
            if(order == null) {
                throw new IllegalStateException("当前桌台没有未支付订单");
            }
            if(!orderService.canCheckout(order.getId())) {
                throw new IllegalStateException("还有菜品未出餐，请等待管理员出餐或取消后再结账");
            }
            //桌台状态 2 = 待结账，检查完毕，可以结账，直接返回订单。
            if(table.getStatus() == 2) {
                return order;
            }
            if(table.getStatus() != 1 || tableMapper.updateStatusIfCurrent(tableId, 1, 2) == 0) {
                throw new IllegalStateException("桌台不是用餐中状态，请刷新后重试");
            }
            return order;
        }
    }

    private void validate(OrderDTO orderDTO) {
        if (orderDTO == null || orderDTO.getTableId() == null) {
            throw new IllegalArgumentException("桌台不能为空");
        }
        if (orderDTO.getItems() == null || orderDTO.getItems().isEmpty()) {
            throw new IllegalArgumentException("订单不能为空");
        }
        orderDTO.getItems().forEach(item -> {
            if (item.getDishId() == null || item.getCount() == null || item.getCount() <= 0) {
                throw new IllegalArgumentException("菜品和数量必须有效");
            }
        });
    }
}
