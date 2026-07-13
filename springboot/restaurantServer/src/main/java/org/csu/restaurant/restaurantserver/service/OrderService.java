package org.csu.restaurant.restaurantserver.service;

import org.csu.restaurant.restaurantserver.dto.OrderDTO;
import org.csu.restaurant.restaurantserver.dto.OrderDetailDTO;
import org.csu.restaurant.restaurantserver.dto.OrderItemDTO;
import org.csu.restaurant.restaurantserver.entity.Order;

public interface OrderService {

   boolean submit(OrderDTO orderDTO);

   Order findUnpaid(Integer tableId);

   OrderDetailDTO findDetailByTableId(Integer tableId);

   Order findById(Integer id);
}
