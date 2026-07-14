package org.csu.restaurant.restaurantserver.service;

import org.csu.restaurant.restaurantserver.dto.OrderDTO;
import org.csu.restaurant.restaurantserver.dto.OrderTaskDTO;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.core.task.TaskRejectedException;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;
import org.csu.restaurant.restaurantserver.entity.DiningTable;
import org.csu.restaurant.restaurantserver.entity.Order;
import org.csu.restaurant.restaurantserver.mapper.TableMapper;

import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Executor;

@Service
public class OrderQueueService {
    private final Executor orderTaskExecutor;
    private final OrderService orderService;
    private final TableMapper tableMapper;
    private final ConcurrentHashMap<String, OrderTaskDTO> tasks = new ConcurrentHashMap<>();
    private final ConcurrentHashMap<Integer, Object> tableLocks = new ConcurrentHashMap<>();
    private final ConcurrentHashMap<Integer, Integer> activeByTable = new ConcurrentHashMap<>();

    public OrderQueueService(@Qualifier("orderTaskExecutor") Executor orderTaskExecutor,
                             OrderService orderService,
                             TableMapper tableMapper) {
        this.orderTaskExecutor = orderTaskExecutor;
        this.orderService = orderService;
        this.tableMapper = tableMapper;
    }

    public OrderTaskDTO enqueue(OrderDTO orderDTO) {
        validate(orderDTO);
        String taskId = UUID.randomUUID().toString();
        OrderTaskDTO queued = new OrderTaskDTO(taskId, "QUEUED", "订单已进入处理队列");
        tasks.put(taskId, queued);
        activeByTable.merge(orderDTO.getTableId(), 1, Integer::sum);

        try {
            orderTaskExecutor.execute(() -> process(taskId, orderDTO));
        } catch (TaskRejectedException ex) {
            tasks.remove(taskId);
            activeByTable.computeIfPresent(orderDTO.getTableId(), (id, count) -> count <= 1 ? null : count - 1);
            throw new IllegalStateException("订单队列繁忙，请稍后重试");
        }
        return queued;
    }

    public OrderTaskDTO getTask(String taskId) {
        OrderTaskDTO task = tasks.get(taskId);
        if (task == null) {
            throw new IllegalArgumentException("订单任务不存在");
        }
        return task;
    }

    private void process(String taskId, OrderDTO orderDTO) {
        tasks.put(taskId, new OrderTaskDTO(taskId, "PROCESSING", "正在处理订单"));
        Object tableLock = tableLocks.computeIfAbsent(orderDTO.getTableId(), id -> new Object());
        synchronized (tableLock) {
            try {
                orderService.submit(orderDTO);
                activeByTable.computeIfPresent(orderDTO.getTableId(),
                        (id, count) -> count <= 1 ? null : count - 1);
                tasks.put(taskId, new OrderTaskDTO(taskId, "SUCCESS", "下单成功"));
            } catch (Exception ex) {
                activeByTable.computeIfPresent(orderDTO.getTableId(),
                        (id, count) -> count <= 1 ? null : count - 1);
                String message = ex.getMessage() == null ? "订单处理失败" : ex.getMessage();
                tasks.put(taskId, new OrderTaskDTO(taskId, "FAILED", message));
            }
        }
    }

    @Transactional
    public Order checkout(Integer tableId) {
        if(tableId == null) {
            throw new IllegalArgumentException("桌台不能为空");
        }
        Object tableLock = tableLocks.computeIfAbsent(tableId, id -> new Object());
        synchronized (tableLock) {
            if(activeByTable.getOrDefault(tableId, 0) > 0) {
                throw new IllegalStateException("还有订单正在后台处理，请稍后再结账");
            }
            DiningTable table = tableMapper.findById(tableId);
            if(table == null) {
                throw new IllegalArgumentException("桌台不存在");
            }
            Order order = orderService.findUnpaid(tableId);
            if(order == null) {
                throw new IllegalStateException("当前桌台没有未支付订单");
            }
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
