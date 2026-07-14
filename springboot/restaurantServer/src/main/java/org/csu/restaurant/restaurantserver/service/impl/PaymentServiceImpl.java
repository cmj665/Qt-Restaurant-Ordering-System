package org.csu.restaurant.restaurantserver.service.impl;

import org.csu.restaurant.restaurantserver.dto.PaymentDTO;
import org.csu.restaurant.restaurantserver.entity.DiningTable;
import org.csu.restaurant.restaurantserver.entity.Order;
import org.csu.restaurant.restaurantserver.entity.Payment;
import org.csu.restaurant.restaurantserver.mapper.OrderMapper;
import org.csu.restaurant.restaurantserver.mapper.PaymentMapper;
import org.csu.restaurant.restaurantserver.mapper.TableMapper;
import org.csu.restaurant.restaurantserver.service.PaymentService;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

@Service
public class PaymentServiceImpl implements PaymentService{

    @Autowired
    private OrderMapper orderMapper;

    @Autowired
    private PaymentMapper paymentMapper;

    @Autowired
    private TableMapper tableMapper;

    @Override
    @Transactional
    public boolean pay(PaymentDTO paymentDTO){

        //1.查询订单
        Order order = orderMapper.findById(paymentDTO.getOrderId());

        if(order==null){
            throw new RuntimeException("订单不存在");
        }
        //订单已支付不能重复支付
        if(order.getPayStatus()==1){

            throw new IllegalStateException("订单已经支付，请勿重复支付");

        }






        //必须占用了才能支付
        DiningTable table = tableMapper.findById(order.getTableId());
        if(table==null){
            throw new RuntimeException("桌台不存在");
        }
        if(table.getStatus()!=2){
            throw  new RuntimeException("当前桌台不是待结账状态");
        }

        //3、保存支付记录

        // Atomically claim the unpaid order. The earlier read is not enough because
        // concurrent requests can both observe payStatus == 0.
        int result = orderMapper.updatePayStatus(order.getId());
        if(result == 0){
            throw new IllegalStateException("订单已经支付，请勿重复支付");
        }

        Payment payment = new Payment();
        payment.setOrderId(order.getId());
        payment.setPayType(paymentDTO.getPayType());
        payment.setPayMoney(order.getTotalPrice());
//        //如果有status字段
//        payment.setStatus(1);

        paymentMapper.insertPayment(payment);



        //4.桌台状态修改
        if(tableMapper.updateStatusIfCurrent(order.getTableId(), 2, 3) == 0){
            throw new IllegalStateException("桌台状态已变化，请刷新后重试");
        }
        return true;
    }

}
