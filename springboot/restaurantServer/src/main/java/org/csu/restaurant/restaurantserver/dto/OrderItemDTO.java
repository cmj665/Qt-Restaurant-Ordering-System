package org.csu.restaurant.restaurantserver.dto;

import lombok.Data;

import java.math.BigDecimal;

@Data
public class OrderItemDTO {
    private Integer id;
    private Integer orderId;
    private Integer tableId;
    private String tableName;
    private Integer dishId;
    private Integer count;
    private String dishName;
    private BigDecimal price;
    private String picture;
    private Integer itemStatus;
}
