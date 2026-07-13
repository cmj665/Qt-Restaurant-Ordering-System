package org.csu.restaurant.restaurantserver.dto;

import lombok.Data;
import java.util.List;

@Data
public class OrderDTO {

    private Integer tableId;

    private List<OrderItemDTO> items;

}
