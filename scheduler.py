#!/usr/bin/env python3
"""
COMPE571 PA3 - Real-time scheduler with DVFS (EDF, RM, EE EDF, EE RM)

Usage:
    python3 scheduler.py <input_file> <EDF|RM> [EE]

Units: times in seconds, power in mW, energy printed in Joules.
Energy of a segment (J) = power(mW) * duration(s) / 1000.
Works on Python 3.9+.
"""
import sys

FREQS = [1188, 918, 648, 384]   # index 0 = fastest, index 3 = slowest


# --------------------------------------------------------------------------
# Input parsing
# --------------------------------------------------------------------------
def parse_input(path):
    with open(path) as f:
        rows = [line.split() for line in f if line.strip()]
    header = rows[0]
    n = int(header[0])
    horizon = int(header[1])                      # system runs up to this time
    power = [int(x) for x in header[2:6]]         # active power per frequency
    idle_power = int(header[6])                   # idle power (lowest freq)
    tasks = []
    for r in rows[1:1 + n]:
        tasks.append({
            "name": r[0],
            "period": int(r[1]),                  # period == deadline
            "wcet": [int(x) for x in r[2:6]],     # WCET at each frequency
        })
    return horizon, power, idle_power, tasks


# --------------------------------------------------------------------------
# Jobs: every task releases a job at k*period with deadline (k+1)*period
# --------------------------------------------------------------------------
class Job:
    def __init__(self, task, k):
        self.name = task["name"]
        self.period = task["period"]
        self.wcet = task["wcet"]
        self.release = k * self.period
        self.deadline = (k + 1) * self.period
        self.freq = None               # chosen frequency index (None = not started)
        self.remaining = self.wcet[0]  # time left at the chosen frequency
        self.finish = None

    def clone(self):
        c = Job.__new__(Job)
        c.__dict__.update(self.__dict__)
        return c


def make_jobs(tasks, horizon):
    jobs = []
    for t in tasks:
        k = 0
        while k * t["period"] < horizon:
            jobs.append(Job(t, k))
            k += 1
    return jobs


# Priority functions: smaller tuple = higher priority
def edf_priority(j):
    return (j.deadline, j.release, j.name)


def rm_priority(j):
    return (j.period, j.release, j.name)


# --------------------------------------------------------------------------
# Core preemptive simulator. Walks through time, always running the
# highest-priority ready job until it finishes or something new is released.
# `chooser(job, now, jobs)` decides the frequency index when a job FIRST starts;
# the job then keeps that frequency even if it is preempted.
# Returns a list of segments: (start, name_or_None, freq_idx_or_None, duration)
# --------------------------------------------------------------------------
def simulate(jobs, start, horizon, priority, chooser):
    now = start
    segs = []
    while now < horizon:
        ready = [j for j in jobs if j.finish is None and j.release <= now]
        future = [j.release for j in jobs if j.finish is None and j.release > now]
        next_release = min(future) if future else horizon

        if not ready:                                   # nothing to run -> idle
            end = min(next_release, horizon)
            segs.append((now, None, None, end - now))
            now = end
            continue

        job = min(ready, key=priority)
        if job.freq is None:                            # first time scheduled
            job.freq = chooser(job, now, jobs)
            job.remaining = job.wcet[job.freq]

        run = min(job.remaining, next_release - now, horizon - now)
        segs.append((now, job.name, job.freq, run))
        now += run
        job.remaining -= run
        if job.remaining == 0:
            job.finish = now
    return segs


def count_misses(jobs, horizon):
    """Jobs whose deadline falls inside the run but that did not finish in time."""
    return sum(1 for j in jobs
               if j.deadline <= horizon and (j.finish is None or j.finish > j.deadline))


# --------------------------------------------------------------------------
# Frequency choosers
# --------------------------------------------------------------------------
def max_freq_chooser(job, now, jobs):
    return 0


def make_ee_chooser(power, idle_power, horizon, priority):
    """
    Energy-efficient chooser. When a job is about to start:
      1. Rank frequencies by the energy the job costs ABOVE idle:
             (active_power - idle_power) * WCET_at_that_freq
         (time spent running is time NOT spent idling, so idle power is
          subtracted; this is why 'lower power' is not always 'lower energy').
      2. Take the cheapest frequency for which a look-ahead simulation (this job
         at that frequency, everything else at max speed) causes no more
         deadline misses than running this job at max speed would.
    """
    def chooser(job, now, jobs):
        def misses_if(f):
            trial = [j.clone() for j in jobs]
            me = trial[jobs.index(job)]
            me.freq = f
            me.remaining = me.wcet[f]
            simulate(trial, now, horizon, priority, max_freq_chooser)
            return count_misses(trial, horizon)

        baseline = misses_if(0)
        order = sorted(range(len(FREQS)),
                       key=lambda f: ((power[f] - idle_power) * job.wcet[f], -f))
        for f in order:
            if misses_if(f) <= baseline:
                return f
        return 0
    return chooser


# --------------------------------------------------------------------------
# Output
# --------------------------------------------------------------------------
def merge(segs):
    """Join back-to-back segments with the same task and frequency."""
    out = []
    for s in segs:
        if out and out[-1][1] == s[1] and out[-1][2] == s[2] \
                and out[-1][0] + out[-1][3] == s[0]:
            p = out[-1]
            out[-1] = (p[0], p[1], p[2], p[3] + s[3])
        else:
            out.append(s)
    return out


def report(segs, jobs, power, idle_power, horizon):
    total_energy = 0.0
    idle_time = 0
    for start, name, f, dur in merge(segs):
        if name is None:
            e = idle_power * dur / 1000.0
            idle_time += dur
            print(f"{start} IDLE IDLE {dur} {e:.3f}J")
        else:
            e = power[f] * dur / 1000.0
            print(f"{start} {name} {FREQS[f]} {dur} {e:.3f}J")
        total_energy += e

    print(f"Total energy consumption: {total_energy:.3f}J")
    print(f"Percentage of time idle: {100.0 * idle_time / horizon:.2f}%")
    print(f"Total system execution time: {horizon} sec")

    late = [j for j in jobs if j.deadline <= horizon
            and (j.finish is None or j.finish > j.deadline)]
    for j in late:
        print(f"WARNING: deadline miss: {j.name} (released {j.release}, deadline {j.deadline})")


# --------------------------------------------------------------------------
def main():
    if len(sys.argv) < 3 or sys.argv[2].upper() not in ("EDF", "RM"):
        print("usage: python3 scheduler.py <input_file> <EDF|RM> [EE]")
        sys.exit(1)

    horizon, power, idle_power, tasks = parse_input(sys.argv[1])
    policy = sys.argv[2].upper()
    energy_efficient = len(sys.argv) > 3 and sys.argv[3].upper() == "EE"

    priority = edf_priority if policy == "EDF" else rm_priority
    chooser = (make_ee_chooser(power, idle_power, horizon, priority)
               if energy_efficient else max_freq_chooser)

    jobs = make_jobs(tasks, horizon)
    segs = simulate(jobs, 0, horizon, priority, chooser)
    report(segs, jobs, power, idle_power, horizon)


if __name__ == "__main__":
    main()
