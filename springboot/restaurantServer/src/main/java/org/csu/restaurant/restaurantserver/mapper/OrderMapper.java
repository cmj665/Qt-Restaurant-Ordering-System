package org.csu.restaurant.restaurantserver.mapper;

import org.apache.ibatis.annotations.Mapper;
import org.apache.ibatis.annotations.Param;
import org.csu.restaurant.restaurantserver.dto.OrderDetailDTO;
import org.csu.restaurant.restaurantserver.entity.Order;
import org.csu.restaurant.restaurantserver.entity.OrderItem;

@Mapper
public interface OrderMapper {

    //创建订单
    int insertOrder(Order order);

    //加入订单详情
    int insertItem(OrderItem item);

    //查询订单
    Order findById(Integer id);

    //修改支付状态
    int updatePayStatus(Integer id);

    //找到未支付的订单
    Order findUnpaid(Integer tableId);

    int addTotalPrice(@Param("id") Integer id, @Param("amount") java.math.BigDecimal amount);
    int subtractTotalPrice(@Param("id") Integer id,@Param("amount") java.math.BigDecimal amount);

    //找到订单详情
    OrderDetailDTO findDetailByTableId(Integer tableId);
    OrderDetailDTO findDetailByOrderId(Integer orderId);

}
