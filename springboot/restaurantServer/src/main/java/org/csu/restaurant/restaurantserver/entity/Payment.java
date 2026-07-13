package org.csu.restaurant.restaurantserver.entity;

import lombok.Data;

import java.math.BigDecimal;
import java.time.LocalDateTime;

@Data
public class Payment {

    private Integer id;
    private Integer orderId;
    //1微信 2支付宝
    private Integer payType;
    private BigDecimal payMoney;
    private LocalDateTime payTime;

}
