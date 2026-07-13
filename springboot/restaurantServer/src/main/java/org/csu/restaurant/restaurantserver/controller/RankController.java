package org.csu.restaurant.restaurantserver.controller;

import org.csu.restaurant.restaurantserver.dto.HotDishDTO;
import org.csu.restaurant.restaurantserver.service.RankService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import java.util.List;

@RestController
@RequestMapping("/rank")
public class RankController {

    @Autowired
    private RankService rankService;

    @GetMapping("/hot")
    public List<HotDishDTO> hot(){

        return rankService.hotRank();
    }

}
