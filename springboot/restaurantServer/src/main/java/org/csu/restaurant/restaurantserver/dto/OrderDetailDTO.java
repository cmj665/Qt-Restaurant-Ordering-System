package org.csu.restaurant.restaurantserver.dto;

import lombok.Data;

import java.math.BigDecimal;
import java.util.List;

@Data
public class OrderDetailDTO {
    private Integer orderId;
    private Integer tableId;
    private BigDecimal totalPrice;
    private Integer payStatus;
    private List<OrderItemDTO> items;

}
