package org.csu.restaurant.restaurantserver.service;


import org.csu.restaurant.restaurantserver.dto.HotDishDTO;

import java.util.List;

public interface RankService {

    List<HotDishDTO> hotRank();
}
