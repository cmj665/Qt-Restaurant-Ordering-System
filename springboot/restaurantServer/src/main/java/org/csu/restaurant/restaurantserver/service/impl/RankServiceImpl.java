package org.csu.restaurant.restaurantserver.service.impl;

import org.csu.restaurant.restaurantserver.dto.HotDishDTO;
import org.csu.restaurant.restaurantserver.mapper.RankMapper;
import org.csu.restaurant.restaurantserver.service.RankService;
import lombok.RequiredArgsConstructor;
import org.springframework.stereotype.Service;

import java.util.List;

@Service
@RequiredArgsConstructor
public class RankServiceImpl implements RankService {
    private final RankMapper rankMapper;

    @Override
    public List<HotDishDTO> hotRank(){
        return rankMapper.hotRank();
    }


}
