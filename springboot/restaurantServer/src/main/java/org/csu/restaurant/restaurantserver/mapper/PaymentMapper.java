package org.csu.restaurant.restaurantserver.mapper;
import org.apache.ibatis.annotations.Mapper;
import org.csu.restaurant.restaurantserver.entity.Payment;

@Mapper
public interface PaymentMapper {

    //添加支付记录
    int insertPayment(Payment payment);

}
