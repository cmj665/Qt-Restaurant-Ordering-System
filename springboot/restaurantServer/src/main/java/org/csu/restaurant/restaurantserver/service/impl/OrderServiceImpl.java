package org.csu.restaurant.restaurantserver.service.impl;

import org.csu.restaurant.restaurantserver.dto.OrderDTO;
import org.csu.restaurant.restaurantserver.dto.OrderDetailDTO;
import org.csu.restaurant.restaurantserver.dto.OrderItemDTO;
import org.csu.restaurant.restaurantserver.entity.Dish;
import org.csu.restaurant.restaurantserver.entity.OrderItem;
import org.csu.restaurant.restaurantserver.entity.Order;
import org.csu.restaurant.restaurantserver.entity.DiningTable;
import org.csu.restaurant.restaurantserver.mapper.DishMapper;
import org.csu.restaurant.restaurantserver.mapper.OrderItemMapper;
import org.csu.restaurant.restaurantserver.mapper.OrderMapper;
import org.csu.restaurant.restaurantserver.service.OrderService;
import org.csu.restaurant.restaurantserver.mapper.TableMapper;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import org.springframework.transaction.annotation.Transactional;

import java.math.BigDecimal;
import java.util.List;


@Service
public class OrderServiceImpl implements OrderService{

    @Autowired
    private OrderMapper orderMapper;

    @Autowired
    private DishMapper dishMapper;

    @Autowired
    private TableMapper tableMapper;

    @Autowired
    private OrderItemMapper orderItemMapper;

    @Override
    @Transactional
    public boolean submit(OrderDTO orderDTO){
        //判断是否为空
        if(orderDTO.getItems()==null
                || orderDTO.getItems().isEmpty()){

            throw new RuntimeException("订单不能为空");
        }

        if(orderDTO.getTableId()==null){

            throw new RuntimeException("桌台不能为空");

        }

        DiningTable table = tableMapper.findById(orderDTO.getTableId());
        if(table == null){
            throw new IllegalArgumentException("桌台不存在");
        }
        if(table.getStatus() != 0 && table.getStatus() != 1){
            throw new IllegalStateException("桌台已进入结账流程，不能继续加菜");
        }

        BigDecimal total=BigDecimal.ZERO;


        //计算价钱
        for(OrderItemDTO dto:orderDTO.getItems()){

            Dish dish =dishMapper.findById(dto.getDishId());

            if(dish==null){
                throw new RuntimeException("菜品不存在");
            }

            //库存不足
            if(dish.getStock()<dto.getCount()){
                //因为 Spring 事务默认：只有抛异常才回滚。
                throw new RuntimeException("库存不足");

            }

            BigDecimal money = dish.getPrice().multiply(new BigDecimal(dto.getCount()));

            total = total.add(money);

        }

        //2、创建订单

        Order order = orderMapper.findUnpaid(orderDTO.getTableId());
        //没有未支付订单（第一次点菜）,已有未支付订单（追加菜品）
        if(order == null){
            order = new Order();
            order.setTableId(orderDTO.getTableId());
            order.setTotalPrice(total);
            orderMapper.insertOrder(order);
        }else if(orderMapper.addTotalPrice(order.getId(), total) == 0){
            throw new IllegalStateException("订单状态已变化，请重新提交");
        }  //数据库没有任何一行数据被修改,订单状态已经变了，当前追加菜品操作不合法，直接抛异常


        //插入订单详情
        for(OrderItemDTO dto:orderDTO.getItems()){

            //扣库存
            int result=dishMapper.reduceStock(dto.getDishId(),dto.getCount());

            if(result==0){
                throw new RuntimeException("库存不足");
            }

            Dish dish = dishMapper.findById(dto.getDishId());

            OrderItem item = new OrderItem();

            item.setOrderId((order.getId()));

            item.setDishId((dto.getDishId()));

            item.setCount(dto.getCount());

            item.setPrice(dish.getPrice());
            item.setItemStatus(0);

            orderMapper.insertItem(item);

        }

        //订单成功后，修改桌台状态
        int result=tableMapper.markDining(orderDTO.getTableId());
        if(result==0){
            throw new RuntimeException("桌台状态更新失败");
        }

        return true;

    }

    @Override
    public Order findUnpaid(Integer tableId){

        return orderMapper.findUnpaid(tableId);

    }

    @Override
    public OrderDetailDTO findDetailByTableId(Integer tableId){
        //查询订单主体
        OrderDetailDTO order = orderMapper.findDetailByTableId(tableId);
        if(order==null)
        {
            return null;
        }

        //查询菜品明细
        List<OrderItemDTO> items = orderItemMapper.findByOrderId(order.getOrderId());

        order.setItems(items);
        return order;

    }

    @Override
    public Order findById(Integer id){

        return orderMapper.findById(id);

    }

    @Override
    public OrderDetailDTO findDetailByOrderId(Integer orderId){
        OrderDetailDTO order=orderMapper.findDetailByOrderId(orderId);
        if(order==null)
            throw new IllegalArgumentException("订单不存在");
        order.setItems(orderItemMapper.findByOrderId(orderId));
        return order;
    }

    @Override
    public List<OrderItemDTO> findPendingOrderItems(){
        return orderItemMapper.findPendingOrderItems();
    }

    @Override
    public boolean canCheckout(Integer orderId){
        return orderItemMapper.countPendingByOrderId(orderId)==0;
    }

    //0未出餐 1已出餐 2已取消
    @Override
    @Transactional
    public void updateItemStatus(Integer itemId,Integer status){
        if(itemId==null || (status!=1 && status!=2))
            throw new IllegalArgumentException("菜品状态参数无效");
        OrderItemDTO item=orderItemMapper.findById(itemId);
        if(item==null)
            throw new IllegalArgumentException("订单菜品不存在");
        Order order=orderMapper.findById(item.getOrderId());
        if(order==null || order.getPayStatus()!=0)
            throw new IllegalStateException("订单已结账，不能修改出餐状态");
        if(item.getItemStatus()!=0)
            throw new IllegalStateException("该菜品已处理，请刷新后重试");
        //返回影响行数 0 → 说明并发下别人已经改了这条菜品，抛出异常，事务回滚
        if(orderItemMapper.updateStatusIfPending(itemId,status)==0)
            throw new IllegalStateException("菜品状态已变化，请刷新后重试");
        if(status==2){
            //算出这道菜的总价
            BigDecimal amount=item.getPrice().multiply(BigDecimal.valueOf(item.getCount()));
            //  订单总金额减去该菜品钱，只有订单未结账（pay_status=0）才允许扣减金额。
            //返回值：SQL 执行后受影响的数据库行数，行数没变说明已结账
            if(orderMapper.subtractTotalPrice(item.getOrderId(),amount)==0)
                throw new IllegalStateException("取消菜品失败");
            // 菜品库存加回，恢复库存
            if(dishMapper.addStock(item.getDishId(),item.getCount())==0)
                throw new IllegalStateException("退回菜品库存失败");
        }
    }

}

