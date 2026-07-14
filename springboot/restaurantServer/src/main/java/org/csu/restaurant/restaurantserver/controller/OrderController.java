package org.csu.restaurant.restaurantserver.controller;


import org.csu.restaurant.restaurantserver.dto.OrderDTO;
import org.csu.restaurant.restaurantserver.dto.OrderDetailDTO;
import org.csu.restaurant.restaurantserver.dto.OrderTaskDTO;
import org.csu.restaurant.restaurantserver.entity.Order;
import org.csu.restaurant.restaurantserver.service.OrderService;
import org.csu.restaurant.restaurantserver.service.OrderQueueService;


import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;
import org.springframework.http.ResponseEntity;

import java.util.HashMap;
import java.util.Map;
import java.util.List;
import org.csu.restaurant.restaurantserver.dto.OrderItemDTO;

@RestController
@RequestMapping("/order")
public class OrderController {

    @Autowired
    private OrderService orderService;

    @Autowired
    private OrderQueueService orderQueueService;

    @PostMapping("/submit")
    public ResponseEntity<OrderTaskDTO> submit(@RequestBody OrderDTO orderDTO){
        return ResponseEntity.accepted().body(orderQueueService.enqueue(orderDTO));
    }

    @GetMapping("/task/{taskId}")
    public OrderTaskDTO task(@PathVariable String taskId){
        return orderQueueService.getTask(taskId);
    }

    @PostMapping("/checkout/{tableId}")
    public Order checkout(@PathVariable Integer tableId){
        return orderQueueService.checkout(tableId);
    }

    @GetMapping("/unpaid/{tableId}")
    public Order findUnpaid(@PathVariable Integer tableId){
        return orderService.findUnpaid(tableId);
    }

    @GetMapping("/detail/{tableId}")
    public OrderDetailDTO detail(@PathVariable Integer tableId){

        return orderService.findDetailByTableId(tableId);

    }

    @GetMapping("/receipt/{orderId}")
    public OrderDetailDTO receipt(@PathVariable Integer orderId){
        return orderService.findDetailByOrderId(orderId);
    }

    @GetMapping("/admin/items")
    public List<OrderItemDTO> adminItems(){ return orderService.findPendingOrderItems(); }

    @PostMapping("/item/{itemId}/status/{status}")
    public String updateItemStatus(@PathVariable Integer itemId,@PathVariable Integer status){
        orderService.updateItemStatus(itemId,status);
        return status==1?"出餐成功":"取消成功";
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
        else if(!orderService.canCheckout(id))
        {
            map.put("canPay",false);
            map.put("message","还有菜品未出餐，请等待全部菜品处理完成");
        }
        else
        {
            map.put("canPay",true);
            map.put("message","可以支付");
        }


        return map;
    }


}
