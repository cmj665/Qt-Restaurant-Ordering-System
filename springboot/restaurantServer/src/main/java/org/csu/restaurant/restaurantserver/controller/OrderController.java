package org.csu.restaurant.restaurantserver.controller;


import org.csu.restaurant.restaurantserver.dto.OrderDTO;
import org.csu.restaurant.restaurantserver.dto.OrderDetailDTO;
import org.csu.restaurant.restaurantserver.entity.Order;
import org.csu.restaurant.restaurantserver.service.OrderService;


import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

import java.util.HashMap;
import java.util.Map;

@RestController
@RequestMapping("/order")
public class OrderController {

    @Autowired
    private OrderService orderService;

    @PostMapping("/submit")
    public String submit(@RequestBody OrderDTO orderDTO){

        try{
            boolean result = orderService.submit(orderDTO);

            return "下单成功";
        }catch (Exception e){
            return e.getMessage();
        }

    }

    @GetMapping("/unpaid/{tableId}")
    public Order findUnpaid(@PathVariable Integer tableId){
        return orderService.findUnpaid(tableId);
    }

    @GetMapping("/detail/{tableId}")
    public OrderDetailDTO detail(@PathVariable Integer tableId){

        return orderService.findDetailByTableId(tableId);

    }

    @GetMapping("/canPay/{id}")
    public Map<String,Object> canPay(
            @PathVariable Integer id
    ){

        Order order = orderService.findById(id);


        Map<String,Object> map = new HashMap<>();


        if(order == null)
        {
            map.put("canPay",false);
            map.put("message","订单不存在");
            return map;
        }


        if(order.getPayStatus()==1)
        {
            map.put("canPay",false);
            map.put("message","订单已经支付");
        }
        else
        {
            map.put("canPay",true);
            map.put("message","可以支付");
        }


        return map;
    }


}
