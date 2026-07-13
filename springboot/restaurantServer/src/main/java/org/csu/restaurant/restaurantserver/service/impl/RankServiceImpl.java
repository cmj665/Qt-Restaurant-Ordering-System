package org.csu.restaurant.restaurantserver.service.impl;

import org.csu.restaurant.restaurantserver.dto.HotDishDTO;
import org.csu.restaurant.restaurantserver.mapper.RankMapper;
import org.csu.restaurant.restaurantserver.service.RankService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.List;

@Service
public class RankServiceImpl implements RankService {

    @Autowired
    private RankMapper rankMapper;

    @Override
    public List<HotDishDTO> hotRank(){
        return rankMapper.hotRank();
    }


}
