
#include "last_branch_predictor.h"

int find_btb_slot(branch_target_buffer* btb ,uint32_t if_branch_inst_address){
    for(int i = 0; i < SLOT_SIZE; i++){
        if(if_branch_inst_address == btb->slot[i].branch_inst_addr){
            return i;
        }
    }
    return -1;
}

/* Code Generation Tool Acknowledgement */
int is_btb_empty(branch_target_buffer* btb){
    return btb->head == btb->tail;
}

int is_btb_full(branch_target_buffer* btb){
    return (btb->tail + 1) % SLOT_SIZE == btb->head;
}

void append_history_to_btb(branch_target_buffer* btb, BTB_slot slot){
    if(is_btb_full(btb)){
        btb->head = (btb->head + 1) % SLOT_SIZE;
    }
    btb->slot[btb->tail] = slot;
    btb->tail = (btb->tail + 1) % SLOT_SIZE;
}


// if there is no history about branch, pipeline just fetch PC + 4. Not Prediction + 4.
branch_prediction get_prediction(branch_target_buffer* btb, uint32_t current_pc){
    branch_prediction ret = {0};
    ret.BTB_index = -1;

    uint32_t index = -1;
    if((index = find_btb_slot(btb,current_pc)) == -1){
        return ret;
    }

    // btb index이 있는 경우
    uint32_t branch_target_address = btb->slot[index].branch_target_addr;
    uint32_t branch_inst_address = btb->slot[index].branch_inst_addr;
    uint32_t isBranch = btb->slot[index].is_branch;

    uint32_t is_branch_predicted = 0;
    if((current_pc == branch_inst_address) && isBranch){
        is_branch_predicted = 1;
    }

    ret.is_branch_predicted = is_branch_predicted;
    ret.BTB_index = index;
    ret.target_address = branch_target_address;

    return ret;
}

// this function will be called in execution stage
//
// isBranch variable catches only branch instruction, the branch prediction don't predict for kinds of jump operation
// this function pass through result of the jump instructions and just update pc
// is
branch_correction update_history_and_status(uint32_t btb_index,
                                            branch_target_buffer* btb,
                                            uint32_t ex_is_branch,
                                            uint32_t current_pc,
                                            uint32_t branch_addr){

    // if the btb doesn't have the history, the btb_index container -1 value.
    branch_correction ret = {0};

    if(btb_index != -1){
        // status change follows state machine from ppt
        // correct case for branch prediction
        if((ex_is_branch == 1)&&(btb->slot[btb_index].is_branch == 1)){

        }
            // mis-prediction
        else if((ex_is_branch == 1)&&(btb->slot[btb_index].is_branch == 0)){
            btb->slot[btb_index].is_branch = 1;
            ret.is_mis_prediction = 1;
            ret.revised_addr = branch_addr;
        }
            // mis-prediction
        else if((ex_is_branch == 0)&&(btb->slot[btb_index].is_branch == 1)){
            btb->slot[btb_index].is_branch = 0;
            ret.is_mis_prediction = 1;
            ret.revised_addr = current_pc + 4;
        }
        else if((ex_is_branch == 0)&&(btb->slot[btb_index].is_branch == 0)){

        }
    }
    else{
        // in this case, branch mis-prediction always taken
        // we need to append this branch history into btb and bht
        ret.is_mis_prediction = 1;
        ret.revised_addr = branch_addr;

        BTB_slot new_history = {0};
        new_history.is_branch = 1;
        new_history.branch_inst_addr = current_pc;
        new_history.branch_target_addr = branch_addr;

        append_history_to_btb(btb, new_history);
    }

    return ret;
}


