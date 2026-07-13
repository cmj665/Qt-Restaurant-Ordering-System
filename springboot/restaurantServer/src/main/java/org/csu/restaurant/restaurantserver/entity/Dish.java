package org.csu.restaurant.restaurantserver.entity;

import lombok.Data;
import java.math.BigDecimal;

@Data
public class Dish {
    private Integer id;
    private Integer catId;
    private String name;
    private BigDecimal price;
    private Integer stock;
    private String picture;
    private String description;
    private Integer isDeleted;
}
