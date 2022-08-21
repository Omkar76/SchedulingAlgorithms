#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <queue>

class GanttDescriptor {
public:
	int pid;
	int start;
	int end;

	GanttDescriptor(int pid, int start, int end)
		: pid(pid), start(start), end(end) {}
};

class PCB {
public:
	int pid;
	int arrival;
	int burst;
	int priority;

	PCB(int pid, int arrival, int burst, int priority = 0) : pid(pid), arrival(arrival), burst(burst), priority(priority) {}
	PCB(const PCB& pcb) : PCB(pcb.pid, pcb.arrival, pcb.burst, pcb.priority) {}
};

bool arrivalTimeComparator(PCB pcb1, PCB pcb2) {
	return pcb1.arrival < pcb2.arrival;
}

class SolvedProc : public PCB {
public:
	int start;
	int end;
	int timeRan = 0;
	SolvedProc(const PCB& pcb, int start, int end) : PCB(pcb), start(start), end(end) {}

};

class Solution {
public:
	std::vector <GanttDescriptor> ganttChart;
	std::vector <SolvedProc> solutionTable;
};


Solution fcfs(std::vector<PCB>& pcbs) {
	std::sort(pcbs.begin(), pcbs.end(), arrivalTimeComparator);
	auto solution = Solution();

	int timeLapsed = 0;

	for (int i = 0; i < pcbs.size();) {

		const PCB pcb = pcbs[i];

		if (pcb.arrival > timeLapsed) {
			solution.ganttChart.push_back(GanttDescriptor(-1, timeLapsed, pcb.arrival));
			timeLapsed = pcb.arrival;
		}

		else {
			solution.ganttChart.emplace_back(pcb.pid, timeLapsed, timeLapsed + pcb.burst);
			solution.solutionTable.push_back(SolvedProc(pcb, timeLapsed, timeLapsed + pcb.burst));
			timeLapsed += pcb.burst;
			i++;
		}
	}
	return solution;
}

Solution rr(std::vector<PCB> pcbs, int quantam) {
	std::sort(pcbs.begin(), pcbs.end(), arrivalTimeComparator);
	Solution solution;
	std::queue<SolvedProc> readyQueue;
	int i = 0, timeLapsed = 0;
	while (i < pcbs.size() || !readyQueue.empty() || timeLapsed == 0) {
		if (i < pcbs.size()) {
			if (pcbs[i].arrival > timeLapsed && readyQueue.empty()) {
				PCB& pcb = pcbs[i];
				solution.ganttChart.emplace_back(-1, timeLapsed, pcb.arrival);
				readyQueue.emplace(pcb, pcb.arrival, -1);
				timeLapsed = pcb.arrival;
				i++;
				continue;
			}

		}

		if (!readyQueue.empty()) {
			auto& proc = readyQueue.front();
			if (proc.start == -1) {
				proc.start = timeLapsed;
			}

			int runTime = std::min(quantam, proc.burst - proc.timeRan);
			solution.ganttChart.emplace_back(proc.pid, timeLapsed, timeLapsed + runTime);
			timeLapsed += runTime;
			proc.timeRan += runTime;
		}

		while (i < pcbs.size() && pcbs[i].arrival <= timeLapsed) {
			readyQueue.emplace(pcbs[i++], -1, -1);
		}

		if (!readyQueue.empty()) {
			auto& proc = readyQueue.front();
			if (proc.burst == proc.timeRan) {
				proc.end = timeLapsed;
				readyQueue.pop();
				solution.solutionTable.push_back(proc);
			}
			else {
				readyQueue.pop();
				readyQueue.push(proc);
			}
		}
	}
	return solution;
}

void ganttPrinter(const std::vector<GanttDescriptor>& chart) {
	for (auto& desc : chart) {
		if (desc.pid == -1) {
			std::cout << "Idle from " << desc.start << " to " << desc.end << "\n";
		}
		else {
			std::cout << "process " << desc.pid << " from " << desc.start << " to " << desc.end << "\n";
		}
	}
}

void printTable(const std::vector<SolvedProc> procs) {
	std::cout << std::setw(5) << "PID" << std::setw(10) << "ARRIVAL" << std::setw(10) << "BURST" << std::setw(10) << "PRORITY" << std::setw(10) << "START" << std::setw(10) << "END" << std::setw(10) << "TAT" << std::setw(10) << "WT" << "\n";

	for (const SolvedProc& proc : procs) {
		std::cout << std::setw(5) << proc.pid << std::setw(10) << proc.arrival << std::setw(10) << proc.burst << std::setw(10) << proc.priority << std::setw(10) << proc.start << std::setw(10) << proc.end << std::setw(10) << (proc.end - proc.arrival) << std::setw(10) << (proc.end - proc.arrival - proc.burst) << "\n";
	}

}

int main() {
	std::vector <PCB> pcbs = { PCB(1, 3, 10), PCB(2, 2, 1), PCB(3,4, 2), PCB(4, 10, 6), PCB(5, 6, 5), PCB(6, 28, 1) };
	//{ {1, 0, 5}, {2, 1, 6 },{3, 2, 3}, {4, 3, 1}, {5, 4, 5}, {6, 6, 4} };
	auto solution = rr(pcbs, 2);

	ganttPrinter(solution.ganttChart);
	std::cout << "\n";
	printTable(solution.solutionTable);
}