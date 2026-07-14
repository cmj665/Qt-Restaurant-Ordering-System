package org.csu.restaurant.restaurantserver.dto;

import lombok.AllArgsConstructor;
import lombok.Data;

@Data
@AllArgsConstructor
public class OrderTaskDTO {
    private String taskId;
    private String status;
    private String message;
}
