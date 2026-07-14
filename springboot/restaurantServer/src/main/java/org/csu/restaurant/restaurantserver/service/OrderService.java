package org.csu.restaurant.restaurantserver.service;

import org.csu.restaurant.restaurantserver.dto.OrderDTO;
import org.csu.restaurant.restaurantserver.dto.OrderDetailDTO;
import org.csu.restaurant.restaurantserver.dto.OrderItemDTO;
import org.csu.restaurant.restaurantserver.entity.Order;
import java.util.List;

public interface OrderService {

   boolean submit(OrderDTO orderDTO);

   Order findUnpaid(Integer tableId);

   OrderDetailDTO findDetailByTableId(Integer tableId);
   OrderDetailDTO findDetailByOrderId(Integer orderId);

   Order findById(Integer id);
   List<OrderItemDTO> findPendingOrderItems();
   void updateItemStatus(Integer itemId,Integer status);
   boolean canCheckout(Integer orderId);
}
