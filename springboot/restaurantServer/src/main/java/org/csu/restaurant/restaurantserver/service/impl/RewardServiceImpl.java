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

    // 奖品、中奖记录数据库操作
    private final RewardMapper rewardMapper;
    //订单查询
    private final OrderMapper orderMapper;
    //安全碎句数，用于概率抽奖
    private final SecureRandom random=new SecureRandom();

    public RewardServiceImpl(RewardMapper rewardMapper,OrderMapper orderMapper){
        this.rewardMapper=rewardMapper;this.orderMapper=orderMapper;
    }

    //根据桌台查抽奖次数
    @Override
    public RewardChanceDTO chancesByTable(Integer tableId){
        if(tableId==null)
            throw new IllegalArgumentException("桌台不能为空");
        Order order=orderMapper.findUnpaid(tableId);
       // 无未支付订单：可抽奖次数=
        if(order==null)
            return new RewardChanceDTO(0,0);
        //每200元获得一次抽奖次数
        int earned=order.getTotalPrice().intValue()/200;
        //已经用的次数
        int used=rewardMapper.countDraws(order.getId());
        ///剩余次数=总获得次数 - 已使用次数，不能小于0
        return new RewardChanceDTO(order.getId(),Math.max(0,earned-used));
    }

    //根据订单 ID 查次数，与上面类似
    @Override
    public RewardChanceDTO chancesByOrder(Integer orderId){
        if(orderId==null)
            throw new IllegalArgumentException("订单不能为空");
        Order order=orderMapper.findById(orderId);
        if(order==null)
            throw new IllegalArgumentException("订单不存在");
        int earned=order.getTotalPrice().intValue()/200;
        int used=rewardMapper.countDraws(orderId);
        return new RewardChanceDTO(orderId,Math.max(0,earned-used));
    }

    //执行抽奖。加事务：
    // 抽奖全过程原子操作，任意异常全部回滚，不会出现「抽中奖品但没记录」「重复抽奖」。
    @Override
    @Transactional
    public RewardResultDTO draw(Integer orderId){
        if(orderId==null)
            throw new IllegalArgumentException("订单不能为空");
        //锁锁定未支付订单，防止并发多人同时抽奖
        Order order=rewardMapper.lockUnpaidOrder(orderId);
        if(order==null)
            throw new IllegalStateException("订单不存在");
        //判断是否还有抽奖次数
        int earned=order.getTotalPrice().intValue()/200;
        int used=rewardMapper.countDraws(orderId);
        if(used>=earned)
            throw new IllegalStateException("该订单没有剩余抽奖次数");
        //读取奖品
        List<Reward> rewards=rewardMapper.findAll();
        //把所有奖品的probability概率权重相加
        int total=rewards.stream().mapToInt(Reward::getProbability).sum();
        if(total<=0)
            throw new IllegalStateException("奖励配置无效");
        //  加权随机抽奖算法
        // 生成 1 ~ total 随机数
        int point=random.nextInt(total)+1;
        Reward selected=null;
        int current=0; //current：循环累加权重，用来划分区间。
        //循环累加权重，判断随机落点落在哪个区间：
        for(Reward reward:rewards){
            current+=reward.getProbability();
            if(point<=current)
            {
                selected=reward;
                break;
            }
        }
        if(selected==null)
            throw new IllegalStateException("抽奖失败，请重试");
        if(rewardMapper.insertOrderReward(orderId,selected.getId())==0)
            throw new IllegalStateException("保存中奖记录失败");
        //返回抽奖结果
        return new RewardResultDTO(selected.getId(),selected.getName(),earned-used-1);
    }
}
