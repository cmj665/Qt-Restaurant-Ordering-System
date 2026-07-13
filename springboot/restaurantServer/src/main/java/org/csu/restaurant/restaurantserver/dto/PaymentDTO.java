package org.csu.restaurant.restaurantserver.dto;

import lombok.Data;

@Data
public class PaymentDTO {
    private Integer orderId;

    //1 微信
    //2 支付宝
    private Integer payType;

}
