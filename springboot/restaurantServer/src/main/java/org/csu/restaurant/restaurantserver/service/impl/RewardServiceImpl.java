package org.csu.restaurant.restaurantserver.service.impl;
import java.security.SecureRandom;
import java.util.List;
import org.csu.restaurant.restaurantserver.dto.RewardChanceDTO;
import org.csu.restaurant.restaurantserver.dto.RewardResultDTO;
import org.csu.restaurant.restaurantserver.entity.Order;
import org.csu.restaurant.restaurantserver.entity.Reward;
import org.csu.restaurant.restaurantserver.mapper.OrderMapper;
import org.csu.restaurant.restaurantserver.mapper.RewardMapper;
import org.csu.restaurant.restaurantserver.service.RewardService;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;
@Service
public class RewardServiceImpl implements RewardService {
    private final RewardMapper rewardMapper; private final OrderMapper orderMapper; private final SecureRandom random=new SecureRandom();
    public RewardServiceImpl(RewardMapper rewardMapper,OrderMapper orderMapper){this.rewardMapper=rewardMapper;this.orderMapper=orderMapper;}
    @Override public RewardChanceDTO chancesByTable(Integer tableId){
        if(tableId==null)throw new IllegalArgumentException("桌台不能为空");
        Order order=orderMapper.findUnpaid(tableId);if(order==null)return new RewardChanceDTO(0,0);
        int earned=order.getTotalPrice().intValue()/200;int used=rewardMapper.countDraws(order.getId());
        return new RewardChanceDTO(order.getId(),Math.max(0,earned-used));
    }
    @Override public RewardChanceDTO chancesByOrder(Integer orderId){
        if(orderId==null)throw new IllegalArgumentException("订单不能为空");
        Order order=orderMapper.findById(orderId);if(order==null)throw new IllegalArgumentException("订单不存在");
        int earned=order.getTotalPrice().intValue()/200;int used=rewardMapper.countDraws(orderId);
        return new RewardChanceDTO(orderId,Math.max(0,earned-used));
    }
    @Override @Transactional public RewardResultDTO draw(Integer orderId){
        if(orderId==null)throw new IllegalArgumentException("订单不能为空");
        Order order=rewardMapper.lockUnpaidOrder(orderId);if(order==null)throw new IllegalStateException("订单不存在");
        int earned=order.getTotalPrice().intValue()/200;int used=rewardMapper.countDraws(orderId);
        if(used>=earned)throw new IllegalStateException("该订单没有剩余抽奖次数");
        List<Reward> rewards=rewardMapper.findAll();int total=rewards.stream().mapToInt(Reward::getProbability).sum();
        if(total<=0)throw new IllegalStateException("奖励配置无效");
        int point=random.nextInt(total)+1;Reward selected=null;int current=0;
        for(Reward reward:rewards){current+=reward.getProbability();if(point<=current){selected=reward;break;}}
        if(selected==null)throw new IllegalStateException("抽奖失败，请重试");
        if(rewardMapper.insertOrderReward(orderId,selected.getId())==0)throw new IllegalStateException("保存中奖记录失败");
        return new RewardResultDTO(selected.getId(),selected.getName(),earned-used-1);
    }
}
