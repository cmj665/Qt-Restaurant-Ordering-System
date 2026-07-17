package org.csu.restaurant.restaurantserver.dto;

import lombok.Data;

import java.math.BigDecimal;
import java.util.List;
import java.time.LocalDateTime;

@Data
/** 面向客户端的订单详情快照，聚合订单主信息及其菜品明细。 */
public class OrderDetailDTO {
    private Integer orderId;
    private Integer tableId;
    private BigDecimal totalPrice;
    private Integer payStatus;
    private List<OrderItemDTO> items;
    private LocalDateTime createTime;
    private LocalDateTime finishTime;

}
