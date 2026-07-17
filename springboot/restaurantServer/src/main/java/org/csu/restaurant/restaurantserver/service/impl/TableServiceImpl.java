package org.csu.restaurant.restaurantserver.service.impl;

import org.csu.restaurant.restaurantserver.entity.DiningTable;
import org.csu.restaurant.restaurantserver.mapper.TableMapper;
import org.csu.restaurant.restaurantserver.service.TableService;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.List;

@Service
public class TableServiceImpl implements TableService{

    @Autowired
    private TableMapper tableMapper;

    @Override
    public List<DiningTable> findAll(){
        return tableMapper.findAll();
    }

    //规定桌台只能0->1->2->3->0
    @Override
    public void changeStatus(Integer id,Integer status){
        if (id == null || status == null || status < 0 || status > 3) {
            throw new IllegalArgumentException("桌台或状态参数无效");
        }
        DiningTable table = tableMapper.findById(id);
        if (table == null) {
            throw new IllegalArgumentException("桌台不存在");
        }

        int current = table.getStatus();
        if (current == status) {
            return;
        }
        boolean allowed = (current == 0 && status == 1)
                || (current == 1 && status == 2)
                || (current == 2 && status == 3)
                || (current == 3 && status == 0);
        if (!allowed || tableMapper.updateStatusIfCurrent(id, current, status) == 0) {
            throw new IllegalStateException("桌台状态已变化，请刷新后重试");
        }
    }

    @Override
    public void clean(Integer id) {
        DiningTable table = tableMapper.findById(id);
        if (table == null) {
            throw new IllegalArgumentException("桌台不存在");
        }
        if (table.getStatus() != 3) {
            throw new IllegalStateException("只有已完成未清理的桌台可以清台");
        }
        if (tableMapper.updateStatusIfCurrent(id, 3, 0) == 0) {
            throw new IllegalStateException("桌台状态已变化，请刷新后重试");
        }
    }

}
