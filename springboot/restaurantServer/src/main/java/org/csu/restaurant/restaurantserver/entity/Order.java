package org.csu.restaurant.restaurantserver.entity;

import lombok.Data;

import java.math.BigDecimal;
import java.time.LocalDateTime;

@Data
/** 订单持久化实体；金额使用 BigDecimal，避免浮点运算引入结算误差。 */
public class Order {

    private Integer id;
    private Integer tableId;
    private BigDecimal totalPrice;
    private Integer payStatus;
    private LocalDateTime createTime;
    private LocalDateTime finishTime;

}
