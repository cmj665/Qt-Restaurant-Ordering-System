package org.csu.restaurant.restaurantserver.service;

import org.csu.restaurant.restaurantserver.dto.OrderDTO;
import org.csu.restaurant.restaurantserver.dto.OrderDetailDTO;
import org.csu.restaurant.restaurantserver.dto.OrderItemDTO;
import org.csu.restaurant.restaurantserver.entity.Order;
import java.util.List;

/** 订单领域服务，定义下单、查询、出餐状态流转及结账校验能力。 */
public interface OrderService {

   boolean submit(OrderDTO orderDTO);

   Order findUnpaid(Integer tableId);

   OrderDetailDTO findDetailByTableId(Integer tableId);
   OrderDetailDTO findDetailByOrderId(Integer orderId);

   Order findById(Integer id);
   List<OrderItemDTO> findPendingOrderItems();
   void updateItemStatus(Integer itemId,Integer status);
   /** 仅当订单内所有订单项均已结束处理时返回 true。 */
   boolean canCheckout(Integer orderId);
}
