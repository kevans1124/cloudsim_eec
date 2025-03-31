// Scheduler.cpp
// CloudSim
//
// Created by ELMOOTAZBELLAH ELNOZAHY on 10/20/24.
//

#include "Interfaces.h"
#include "Scheduler.hpp"
#include <climits>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>




// Global state variables
//static bool migrating = true; 
static vector<VMId_t> vms;
static vector<MachineId_t> machines;
static unordered_map <TaskId_t, VMId_t> task_to_vms;
static unordered_set<VMId_t> vms_migrating;
static unsigned active_machines;



void Scheduler::Init() {
    // Initialize the scheduler by retrieving total machines in the system
    unsigned total_machines = Machine_GetTotal();
    active_machines = total_machines;

    for(unsigned i = 0; i < active_machines; i++) {
        MachineId_t machine_id = i;
        machines.push_back(machine_id);
    }

    // Create and attach VMs based on each machine's CPU type and GPU requirements
    for(auto machine_id : machines) {
        MachineInfo_t machine_info = Machine_GetInfo(machine_id);

        VMType_t vm_type = (machine_info.cpu == POWER) ? AIX : LINUX;

        // Handle VM creation based on GPU availability
        VMId_t vm_id = VM_Create(vm_type, machine_info.cpu);
        vms.push_back(vm_id);
        VM_Attach(vm_id, machine_id);
        SimOutput("Init(): VM " + to_string(vm_id) + " created and attached to Machine " + to_string(machine_id), 3);
    }
}

void Scheduler::NewTask(Time_t now, TaskId_t task_id) {
    TaskInfo_t task_info = GetTaskInfo(task_id);
    unsigned task_memory = GetTaskMemory(task_id);

    // Determine task priority based on SLA requirements
    Priority_t priority = LOW_PRIORITY;
    switch(task_info.required_sla) {
        case SLA0: priority = HIGH_PRIORITY; break;
        case SLA1: priority = MID_PRIORITY; break;
        case SLA2: priority = MID_PRIORITY; break;
        case SLA3: priority = LOW_PRIORITY; break;
    }

    // Find existing VM that meets requirements
    for(auto vm_id : vms) {
        VMInfo_t vm_info = VM_GetInfo(vm_id);
        MachineInfo_t machine_info = Machine_GetInfo(vm_info.machine_id);

        // Check if VM matches task's requirements
        if(vm_info.vm_type != task_info.required_vm ||
           vm_info.cpu != task_info.required_cpu ||
           (task_info.gpu_capable && !machine_info.gpus) ||
           machine_info.memory_size - machine_info.memory_used < task_memory) {
            continue;
        }

        // Assign task
        VM_AddTask(vm_id, task_id, priority);
        task_to_vms[task_id] = vm_id;
        return;
    }

    // If no suitable VM, create a new one on first compatible machine
    for(auto machine_id : machines) {
        MachineInfo_t machine_info = Machine_GetInfo(machine_id);
        
        if(machine_info.cpu == task_info.required_cpu &&
           (!task_info.gpu_capable || machine_info.gpus) &&
           machine_info.memory_size - machine_info.memory_used >= task_memory) {
            
            VMId_t vm_new = VM_Create(task_info.required_vm, task_info.required_cpu);
            VM_Attach(vm_new, machine_id);
            vms.push_back(vm_new);
            VM_AddTask(vm_new, task_id, priority);
            task_to_vms[task_id] = vm_new;
            return;
        }
    }
}

void Scheduler::MigrationComplete(Time_t time, VMId_t vm_id) {
}

void Scheduler::PeriodicCheck(Time_t now) {

}

void Scheduler::Shutdown(Time_t time) {
    // Do your final reporting and bookkeeping here.
    // Report about the total energy consumed
    // Report about the SLA compliance
    // Shutdown everything to be tidy :-)
    for(auto & vm: vms) {
        VM_Shutdown(vm);
    }
    SimOutput("SimulationComplete(): Finished!", 4);
    SimOutput("SimulationComplete(): Time is " + to_string(time), 4);
}

void Scheduler::TaskComplete(Time_t now, TaskId_t task_id) {
    auto task = task_to_vms.find(task_id);
    if(task != task_to_vms.end()) {
        task_to_vms.erase(task);
    }
}

// Public interface below

static Scheduler Scheduler;

void InitScheduler() {
    SimOutput("InitScheduler(): Initializing scheduler", 4);
    Scheduler.Init();
}

void HandleNewTask(Time_t time, TaskId_t task_id) {
    SimOutput("HandleNewTask(): Received new task " + to_string(task_id) + " at time " + to_string(time), 4);
    Scheduler.NewTask(time, task_id);
}

void HandleTaskCompletion(Time_t time, TaskId_t task_id) {
    SimOutput("HandleTaskCompletion(): Task " + to_string(task_id) + " completed at time " + to_string(time), 4);
    Scheduler.TaskComplete(time, task_id);
}

void MemoryWarning(Time_t time, MachineId_t machine_id) {
    SimOutput("MemoryWarning(): Overflow at " + to_string(machine_id) + " was detected at time " + to_string(time), 0);
}

void MigrationDone(Time_t time, VMId_t vm_id) {
    // Log migration completion and remove VM from migration list
    SimOutput("MigrationDone(): Migration of VM " + to_string(vm_id) + " completed at time " + to_string(time), 4);
    vms_migrating.erase(vm_id);
    Scheduler.MigrationComplete(time, vm_id);
}

void SchedulerCheck(Time_t time) {
    SimOutput("SchedulerCheck(): SchedulerCheck() called at " + to_string(time), 4);
    Scheduler.PeriodicCheck(time);
    static unsigned counts = 0;
    counts++;

}

void SimulationComplete(Time_t time) {
    cout << "SLA violation report" << endl;
    cout << "SLA0: " << GetSLAReport(SLA0) << "%" << endl;
    cout << "SLA1: " << GetSLAReport(SLA1) << "%" << endl;
    cout << "SLA2: " << GetSLAReport(SLA2) << "%" << endl;
    cout << "Total Energy " << Machine_GetClusterEnergy() << "KW-Hour" << endl;
    cout << "Simulation run finished in " << double(time)/1000000 << " seconds" << endl;
    SimOutput("SimulationComplete(): Simulation finished at time " + to_string(time), 4);
    
    Scheduler.Shutdown(time);
}

void SLAWarning(Time_t time, TaskId_t task_id) {

}

void StateChangeComplete(Time_t time, MachineId_t machine_id) {
    // Called in response to an earlier request to change the state of a machine
}