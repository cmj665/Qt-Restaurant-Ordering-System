package org.csu.restaurant.restaurantserver.mapper;

import org.apache.ibatis.annotations.Mapper;
import org.apache.ibatis.annotations.Param;
import org.csu.restaurant.restaurantserver.dto.OrderItemDTO;

import java.util.List;

@Mapper
public interface OrderItemMapper {
    // //根据订单id查询菜品明细
    List<OrderItemDTO> findByOrderId(Integer orderId);
    List<OrderItemDTO> findPendingOrderItems();
    OrderItemDTO findById(Integer id);
    int updateStatusIfPending(@Param("id") Integer id,@Param("status") Integer status);
    int countPendingByOrderId(Integer orderId);
}
