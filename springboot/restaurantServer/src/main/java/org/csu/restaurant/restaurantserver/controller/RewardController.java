package org.csu.restaurant.restaurantserver.controller;
import org.csu.restaurant.restaurantserver.dto.RewardChanceDTO;
import org.csu.restaurant.restaurantserver.dto.RewardResultDTO;
import org.csu.restaurant.restaurantserver.service.RewardService;
import org.springframework.web.bind.annotation.*;
@RestController
@RequestMapping("/reward")
public class RewardController {
    private final RewardService rewardService;
    public RewardController(RewardService rewardService){this.rewardService=rewardService;}
    @GetMapping("/chances/table/{tableId}") public RewardChanceDTO chances(@PathVariable Integer tableId){return rewardService.chancesByTable(tableId);}
    @GetMapping("/chances/order/{orderId}") public RewardChanceDTO orderChances(@PathVariable Integer orderId){return rewardService.chancesByOrder(orderId);}
    @PostMapping("/draw/{orderId}") public RewardResultDTO draw(@PathVariable Integer orderId){return rewardService.draw(orderId);}
}
