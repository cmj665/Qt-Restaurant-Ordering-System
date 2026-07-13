package org.csu.restaurant.restaurantserver.mapper;

import org.apache.ibatis.annotations.Mapper;
import org.csu.restaurant.restaurantserver.dto.HotDishDTO;

import java.util.List;

@Mapper
public interface RankMapper {

    List<HotDishDTO> hotRank();

}
