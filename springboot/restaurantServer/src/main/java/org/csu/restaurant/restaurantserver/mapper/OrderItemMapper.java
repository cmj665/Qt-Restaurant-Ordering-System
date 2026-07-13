package org.csu.restaurant.restaurantserver.mapper;

import org.apache.ibatis.annotations.Mapper;
import org.csu.restaurant.restaurantserver.dto.OrderItemDTO;

import java.util.List;

@Mapper
public interface OrderItemMapper {
    // //根据订单id查询菜品明细
    List<OrderItemDTO> findByOrderId(Integer orderId);
}
