package org.csu.restaurant.restaurantserver.service.impl;

import org.csu.restaurant.restaurantserver.dto.OrderDTO;
import org.csu.restaurant.restaurantserver.dto.OrderDetailDTO;
import org.csu.restaurant.restaurantserver.dto.OrderItemDTO;
import org.csu.restaurant.restaurantserver.entity.Dish;
import org.csu.restaurant.restaurantserver.entity.OrderItem;
import org.csu.restaurant.restaurantserver.entity.Order;
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

        Order order=new Order();

        order.setTableId(orderDTO.getTableId());

        order.setTotalPrice(total);

        //插入订单
        orderMapper.insertOrder(order);


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

            orderMapper.insertItem(item);

        }

        //订单成功后，修改桌台状态
        int result=tableMapper.updateStatus(orderDTO.getTableId(),1);
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

}

