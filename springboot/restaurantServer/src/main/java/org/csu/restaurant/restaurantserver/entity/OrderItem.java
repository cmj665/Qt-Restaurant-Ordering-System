package org.csu.restaurant.restaurantserver.entity;

import lombok.Data;

import java.math.BigDecimal;

@Data
public class OrderItem {

    private Integer id;
    private Integer orderId;
    private Integer dishId;
    private Integer count;
    private BigDecimal price;

}
