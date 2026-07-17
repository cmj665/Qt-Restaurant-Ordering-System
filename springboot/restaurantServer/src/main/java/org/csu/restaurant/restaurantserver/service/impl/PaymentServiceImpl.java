package org.csu.restaurant.restaurantserver.service.impl;

import org.csu.restaurant.restaurantserver.dto.PaymentDTO;
import org.csu.restaurant.restaurantserver.entity.DiningTable;
import org.csu.restaurant.restaurantserver.entity.Order;
import org.csu.restaurant.restaurantserver.entity.Payment;
import org.csu.restaurant.restaurantserver.mapper.OrderMapper;
import org.csu.restaurant.restaurantserver.mapper.PaymentMapper;
import org.csu.restaurant.restaurantserver.mapper.TableMapper;
import org.csu.restaurant.restaurantserver.mapper.OrderItemMapper;
import org.csu.restaurant.restaurantserver.service.PaymentService;
import lombok.RequiredArgsConstructor;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

@Service
@RequiredArgsConstructor
public class PaymentServiceImpl implements PaymentService{
    private final OrderMapper orderMapper;
    private final PaymentMapper paymentMapper;
    private final TableMapper tableMapper;
    private final OrderItemMapper orderItemMapper;

    @Override
    @Transactional   //把整个pay()方法包含的数据库操作放进同一个事务中。
    public boolean pay(PaymentDTO paymentDTO){

        //1.查询订单
        Order order = orderMapper.findById(paymentDTO.getOrderId());

        if(order==null){
            throw new RuntimeException("订单不存在");
        }
        //订单已支付不能重复支付，pay_status = 1：已支付
        if(order.getPayStatus()==1){

            throw new IllegalStateException("订单已经支付，请勿重复支付");

        }

        //必须占用桌台了才能支付
        DiningTable table = tableMapper.findById(order.getTableId());
        if(table==null){
            throw new RuntimeException("桌台不存在");
        }
        if(table.getStatus()!=2){
            throw  new RuntimeException("当前桌台不是待结账状态");
        }


        //检查菜品是否全部处理完成
        if(orderItemMapper.countPendingByOrderId(order.getId()) > 0){
            throw new IllegalStateException("还有菜品未出餐，暂时不能支付");
        }

        //3、保存支付记录
        // Atomically claim the unpaid order. The earlier read is not enough because
        // concurrent requests can both observe payStatus == 0.
        //防止重复支付的原子更新,真正的原子更新SQL：AND pay_status = 0(OrderMapper.xml)
        int result = orderMapper.updatePayStatus(order.getId());
        if(result == 0){
            throw new IllegalStateException("订单已经支付，请勿重复支付");
        }
        //保存支付记录,支付记录只有在订单状态原子更新成功后才会插入,即支付记录在抢占成功后才保存
        Payment payment = new Payment();
        payment.setOrderId(order.getId());
        payment.setPayType(paymentDTO.getPayType());
        payment.setPayMoney(order.getTotalPrice());

        paymentMapper.insertPayment(payment);

        //4.桌台状态修改,只有桌台当前状态为2，才能修改成3，即支付完成后，由待支付状态2转变为未清理状态3
        if(tableMapper.updateStatusIfCurrent(order.getTableId(), 2, 3) == 0){
            throw new IllegalStateException("桌台状态已变化，请刷新后重试");
        }
        return true;
    }

}
