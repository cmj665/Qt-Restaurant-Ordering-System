package org.csu.restaurant.restaurantserver.config;

//@Bean：把方法返回对象交给 Spring 容器管理，可通过名称注入
import org.springframework.context.annotation.Bean;
//@Configuration：标识当前类是 Spring 配置类，项目启动自动扫描加载
import org.springframework.context.annotation.Configuration;
import org.springframework.scheduling.concurrent.ThreadPoolTaskExecutor;

import java.util.concurrent.Executor;
import java.util.concurrent.ThreadPoolExecutor;

//线程池配置
@Configuration
public class OrderQueueConfig {

    //注册一个 Bean，Bean 名称为 orderTaskExecutor
    @Bean("orderTaskExecutor")
    public Executor orderTaskExecutor() {
        //创建 Spring 线程池实例
        ThreadPoolTaskExecutor executor = new ThreadPoolTaskExecutor();
        //正常有4个后台线程，繁忙时最多增加到8个线程；
        //等待队列最多存放100个任务；后台线程名称以order-worker-开头。
        executor.setCorePoolSize(4);
        executor.setMaxPoolSize(8);
        executor.setQueueCapacity(100);
        //非核心线程空闲存活时间
        executor.setKeepAliveSeconds(60);
        executor.setThreadNamePrefix("order-worker-");
        // 任务拒绝策略。AbortPolicy：默认拒绝策略
        executor.setRejectedExecutionHandler(new ThreadPoolExecutor.AbortPolicy());
        //关闭服务时等待任务执行完成，防止下单中途中断导致数据错乱。
        executor.setWaitForTasksToCompleteOnShutdown(true);
        //最大等待关闭时长
        executor.setAwaitTerminationSeconds(30);
        //初始化线程池
        executor.initialize();
        return executor;
    }
}
