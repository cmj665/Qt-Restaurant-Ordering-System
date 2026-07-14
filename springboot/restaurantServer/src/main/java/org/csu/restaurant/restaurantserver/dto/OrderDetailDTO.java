package org.csu.restaurant.restaurantserver.dto;

import lombok.Data;

import java.math.BigDecimal;
import java.util.List;
import java.time.LocalDateTime;

@Data
public class OrderDetailDTO {
    private Integer orderId;
    private Integer tableId;
    private BigDecimal totalPrice;
    private Integer payStatus;
    private List<OrderItemDTO> items;
    private LocalDateTime createTime;
    private LocalDateTime finishTime;

}
