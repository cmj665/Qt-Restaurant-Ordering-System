package org.csu.restaurant.restaurantserver.entity;

import lombok.Data;

import java.math.BigDecimal;
import java.time.LocalDateTime;

@Data
public class Order {

    private Integer id;
    private Integer tableId;
    private BigDecimal totalPrice;
    private Integer payStatus;
    private LocalDateTime createTime;
    private LocalDateTime finishTime;

}
