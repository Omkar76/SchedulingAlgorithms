#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <queue>
#include <functional>

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

	PCB(int pid, int arrival, int burst, int priority = 0) : 
		pid(pid), arrival(arrival), burst(burst), priority(priority) {}
	PCB(const PCB& pcb) : PCB(pcb.pid, pcb.arrival, pcb.burst, pcb.priority) {}
};

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

auto arrivalTimeComparator = [](const PCB& pcb1, const PCB& pcb2) {
	return pcb1.arrival < pcb2.arrival;
};

auto arrivalAndBurstComparator = [](const PCB& pcb1, const PCB& pcb2) {
	if (pcb1.arrival < pcb2.arrival) return true;
	if (pcb2.arrival < pcb1.arrival) return false;

	return pcb1.burst < pcb2.burst;
};

// assuming lower the number, higher the priority 
// for some reason priority queue is accepting the the reverse of comparator required by std::sort
// if the same comparator is used in std::sort it'll sort descending
// using it in queue however gives elements in ascending order
// TODO: Add explaination about it here. Leave it for now :skull:
auto priorityComparator = [](const PCB& pcb1, const PCB& pcb2) {

	if (pcb1.priority > pcb2.priority) return true;
	if (pcb2.priority > pcb1.priority) return false;

	return pcb1.arrival > pcb2.arrival;
};

auto shortestJobComparator = [](const PCB& pcb1, const PCB& pcb2) {
	if (pcb1.burst > pcb2.burst) return true;
	if (pcb2.burst > pcb1.burst) return false;

	return pcb1.arrival > pcb2.arrival;
};

Solution fcfs(std::vector<PCB>& pcbs) {
	std::sort(pcbs.begin(), pcbs.end(), arrivalTimeComparator);
	Solution solution;

	int timeLapsed = 0;

	for (int i = 0; i < pcbs.size();) {
		const PCB pcb = pcbs[i];

		if (pcb.arrival > timeLapsed) {
			solution.ganttChart.emplace_back(-1, timeLapsed, pcb.arrival);
			timeLapsed = pcb.arrival;
		}
		else {
			solution.ganttChart.emplace_back(pcb.pid, timeLapsed, timeLapsed + pcb.burst);
			solution.solutionTable.emplace_back(pcb, timeLapsed, timeLapsed + pcb.burst);
			timeLapsed += pcb.burst;
			i++;
		}
	}
	return solution;
}


Solution hpf(std::vector<PCB>& pcbs, std::function<bool(const PCB&, const PCB&)> comparator = priorityComparator) {
	std::sort(pcbs.begin(), pcbs.end(), arrivalTimeComparator);

	Solution solution;

	std::priority_queue<SolvedProc, std::vector<SolvedProc>, decltype(comparator)> readyQueue(comparator);
	int i = 0, timeLapsed = 0;

	while (i < pcbs.size() || !readyQueue.empty() || timeLapsed == 0) {
		if (i < pcbs.size()) {
			PCB& pcb = pcbs[i];
			if (pcb.arrival > timeLapsed && readyQueue.empty()) {
				solution.ganttChart.emplace_back(-1, timeLapsed, pcb.arrival);
				readyQueue.emplace(pcb, -1, -1);
				timeLapsed = pcb.arrival;
				i++;
				continue;
			}
		}

		while (i < pcbs.size() && pcbs[i].arrival <= timeLapsed) {
			readyQueue.emplace(pcbs[i++], -1, -1);
		}

		if (!readyQueue.empty()) {
			auto& proc = readyQueue.top();
			solution.ganttChart.emplace_back(proc.pid, timeLapsed, timeLapsed + proc.burst);
			SolvedProc solvedProc(proc);
			solvedProc.start = timeLapsed;
			solvedProc.end = timeLapsed + proc.burst;
			solution.solutionTable.push_back(solvedProc);
			timeLapsed += proc.burst;
			readyQueue.pop();
		}

	}
	return solution;
}

Solution sjf(std::vector<PCB>& pcbs) {
	std::sort(pcbs.begin(), pcbs.end(), arrivalAndBurstComparator);
	return hpf(pcbs, shortestJobComparator);
}

/*Solution _preemptiveHpf(std::vector<PCB> pcbs, std::function<bool(const PCB&, const PCB&)> comparator = priorityComparator){
	std::sort(pcbs.begin(), pcbs.end(), arrivalTimeComparator);

	Solution solution;
	std::priority_queue<SolvedProc, std::vector<SolvedProc>, decltype(comparator)> readyQueue(comparator);
	int i = 0, timeLapsed = 0;


	while (i < pcbs.size() && !readyQueue.empty()) {
		while(i < pcbs.size() && pcbs[i].arrival == timeLapsed) {
			readyQueue.emplace(pcbs[i], -1, -1);
			i++;
		}
		
		if (readyQueue.empty()) {
			timeLapsed++; continue;
		}

		auto _proc = readyQueue.top();
		readyQueue.pop();

		SolvedProc proc(_proc);

		if (proc.start == -1) {
			proc.start = timeLapsed;
		}

		proc.timeRan++;

		if (proc.timeRan != proc.burst) {
			readyQueue.push(proc);
		}
		else {
			proc.end = timeLapsed;
			solution.solutionTable.push_back(proc);
		}
		
		timeLapsed++;
	}
	return solution;
}
*/

Solution preemptiveHpf(std::vector<PCB> pcbs, std::function<bool(const PCB&, const PCB&)> comparator = priorityComparator) {
	std::sort(pcbs.begin(), pcbs.end(), arrivalTimeComparator);
	//for (auto p : pcbs) {
	//	std::cout << p.pid << " " << p.arrival << " " << p.burst << "\n";
	//}
	Solution solution;
	std::priority_queue<SolvedProc, std::vector<SolvedProc>, decltype(comparator)> readyQueue(comparator);
	int i = 0, timeLapsed = 0;

	while (i < pcbs.size() || ! readyQueue.empty() || timeLapsed == 0) {
		if (i < pcbs.size()) {
			PCB& pcb = pcbs[i];

			if (pcb.arrival > timeLapsed && readyQueue.empty()) {
				solution.ganttChart.emplace_back(-1, timeLapsed, pcb.arrival);
				readyQueue.emplace(pcb, -1, -1);
				timeLapsed = pcb.arrival;
				i++;
				continue;
			}
		}
		
		while (i < pcbs.size() && pcbs[i].arrival == timeLapsed) {
			readyQueue.emplace(pcbs[i], -1, -1);
			i++;
		}
		
		if (!readyQueue.empty()) {
			//std::cout << timeLapsed << " ";
			auto _proc = readyQueue.top();// is const ref, cant be modified
			std::cout << _proc.pid << "-";
			readyQueue.pop();
			SolvedProc proc(_proc);		//create a mutable instance from it
			std::cout << proc.pid << " ";						// all modifications to proc must be done before pushing it into queue!
										// modifying after push has no effect on pushed value!!!!!!!!!!!

			if (proc.start == -1) {
				proc.start = timeLapsed;
			}


			int timeTillNextProcArrival = std::numeric_limits<int>::max();
			if(i < pcbs.size()) {
				timeTillNextProcArrival = pcbs[i].arrival - timeLapsed;
				//std::cout << pcbs[i].pid << " " << timeTillNextProcArrival << " " << i << "\n";
			}

			int timeToFinish = proc.burst - proc.timeRan;

			int timeSlice = std::min(timeToFinish, timeTillNextProcArrival);

			solution.ganttChart.emplace_back(proc.pid, timeLapsed, timeLapsed + timeSlice);

			timeLapsed += timeSlice;
			proc.timeRan += timeSlice;

			if (proc.burst == proc.timeRan) {
				proc.end = timeLapsed;
				solution.solutionTable.push_back(proc);
			}
			else {
				readyQueue.push(proc);
			}
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
			PCB& pcb = pcbs[i];
			if (pcb.arrival > timeLapsed && readyQueue.empty()) {
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

void printTable(const std::vector<SolvedProc>& procs) {
	std::cout << std::setw(5) << "PID" << std::setw(10) << "ARRIVAL" << std::setw(10) << "BURST" << std::setw(10) << "PRORITY" << std::setw(10) << "START" << std::setw(10) << "END" << std::setw(10) << "TAT" << std::setw(10) << "WT" << "\n";

	for (const SolvedProc& proc : procs) {
		std::cout << std::setw(5) << proc.pid << std::setw(10) << proc.arrival << std::setw(10) << proc.burst << std::setw(10) << proc.priority << std::setw(10) << proc.start << std::setw(10) << proc.end << std::setw(10) << (proc.end - proc.arrival) << std::setw(10) << (proc.end - proc.arrival - proc.burst) << "\n";
	}
}

int main() {
	std::vector <PCB> pcbs
	{ {1, 3, 10, 5 }, { 2, 2, 1, 1}, { 3,4, 2,3 },{ 4, 10, 6, 4 }, { 5, 6, 5, 2 }, { 6, 28, 1, 0 } };

	////{ {1, 0, 4, 2}, {2,1,2,4},{3, 2, 3, 6 }, {4,3, 5, 10},{5,4, 1, 8},{6, 5, 4, 12},{7, 6, 6, 9} };
	////{ {1, 0, 3,2}, {2, 2, 5, 6}, {3, 1, 4, 3},{4, 4, 2, 5 }, {5, 6, 9, 7}, {6, 4, 5, 4 }, {7,10, 7, 10} };

	////{ {1, 0, 4, 1}, {2, 0, 3,2}, {3, 6, 7, 1}, {4, 11, 4, 3}, {5, 12, 2, 2} };

	////{ {1, 0, 5}, {2, 1, 6 },{3, 2, 3}, {4, 3, 1}, {5, 4, 5}, {6, 6, 4} };
	auto solution = preemptiveHpf(pcbs);

	ganttPrinter(solution.ganttChart);
	std::cout << "\n";
	printTable(solution.solutionTable);

	//PCB pcb(1, 2, 5, 9);
	//SolvedProc proc(pcb, 3, 8);

	//SolvedProc proc1(proc);

	//std::cout << proc1.pid << " " << proc1.arrival << " " << proc1.burst << " " << proc1.priority << " " << proc1.start << " " << proc1.end;
}