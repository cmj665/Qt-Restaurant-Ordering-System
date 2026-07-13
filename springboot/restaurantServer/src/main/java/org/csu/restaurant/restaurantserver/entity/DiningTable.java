package org.csu.restaurant.restaurantserver.entity;

import lombok.Data;

@Data
public class DiningTable {

    private Integer id;
    private String tableName;
    private Integer capacity;
    private Integer status;

}
