#include "cpu/cpu.h"
#include "disk/disk.h"
#include "http/server.h"
#include "memory/memory.h"
#include "net/net.h"
#include "sensors/sensors.h"
#include "sys/sys.h"
#include <stdio.h>
#include <stdlib.h>

void indexRedirect(Request* req, Response* res)
{
    responseSendStatus(res, HTTP_MOVED_PERMANENTLY);
    responseSetHeader(res, "location", "/index.html");
}

void cpuCount(Request* req, Response* res)
{
    char buffer[64];
    int logical = cpu_count(true);
    int physical = cpu_count(false);
    sprintf(buffer,
        "{\"logical\": \"%d\",\"physical\": \"%d\"}", logical, physical);

    responseSetHeader(res, "content-type", "application/json");
    responseSendBytes(res, buffer, strlen(buffer));
}

void cpuFreq(Request* req, Response* res)
{
    cpu_freq_t freq;
    char buffer[128] = { 0 };
    cpu_freq(&freq);

    sprintf(buffer,
        "{\"min\": \"%.0f\",\"current\": \"%.0f\",\"max\": \"%.0f\"}",
        freq.min, freq.current, freq.max);

    responseSetHeader(res, "content-type", "application/json");
    responseSendBytes(res, buffer, strlen(buffer));
}

void cpuInfo(Request* req, Response* res)
{
    cpu_info_t info;
    cpu_info(&info);
    char buffer[2048] = { 0 };

    sprintf(buffer,
        "{\"arch\": \"%s\","
        "\"vendorStr\": \"%s\","
        "\"brandStr\": \"%s\","
        "\"vendor\": \"%s\","
        "\"family\": \"%d\","
        "\"model\": \"%d\","
        "\"numCores\": \"%d\","
        "\"numLogicalCpus\": \"%d\","
        "\"totalLogicalCpus\": \"%d\","
        "\"l1Cache\": \"%d\","
        "\"l2Cache\": \"%d\","
        "\"l3Cache\": \"%d\","
        "\"l4Cache\": \"%d\","
        "\"codename\": \"%s\","
        "\"purpose\": \"%s\","
        "\"clockByOs\": \"%d\","
        "\"measuredClock\": \"%d\"}",
        info.arch, info.vendor_str, info.brand_str,
        info.vendor, info.family, info.model,
        info.num_cores, info.num_logical_cpus, info.total_logical_cpus,
        info.l1_cache, info.l2_cache, info.l3_cache,
        info.l4_cache, info.codename, info.purpose,
        info.clock_by_os, info.measured_clock);

    responseSetHeader(res, "content-type", "application/json");
    responseSendBytes(res, buffer, strlen(buffer));
}

void cpuLoadAvg(Request* req, Response* res)
{
    loadavg_t avg[3];
    char buffer[128] = { 0 };

    cpu_loadavg(avg);

    sprintf(buffer,
        "{\"one\": \"%0.2f\","
        "\"five\": \"%0.2f\","
        "\"fifteen\": \"%0.2f\"}",
        avg[0], avg[1], avg[2]);

    responseSetHeader(res, "content-type", "application/json");
    responseSendBytes(res, buffer, strlen(buffer));
}

void cpuPercent(Request* req, Response* res)
{
    // Blocking operation for now
    // ?? maybe switch to usleep for more precise intervals
    float usage = cpu_percent(1);
    char buffer[128] = { 0 };
    sprintf(buffer, "{\"cpuPercent\": \"%.02f\"}", usage);

    responseSetHeader(res, "content-type", "application/json");
    responseSendBytes(res, buffer, strlen(buffer));
}

void cpuStats(Request* req, Response* res)
{
    char buffer[128] = { 0 };
    cpu_stats_t stats;

    cpu_stats(&stats);

    sprintf(buffer,
        "{\"ctxSwitches\": \"%llu\","
        "\"interrupts\": \"%llu\","
        "\"softInterrupts\": \"%llu\"}",
        stats.ctx_switches, stats.interrupts, stats.soft_interrupts);

    responseSetHeader(res, "content-type", "application/json");
    responseSendBytes(res, buffer, strlen(buffer));
}

void cpuTimesPercent(Request* req, Response* res)
{
    char buffer[256] = { 0 };
    cpu_times_t tperc;
    cpu_times_percent(&tperc);

    sprintf(buffer,
        "{\"user\": \"%.02Lf\","
        "\"nice\": \"%.02Lf\","
        "\"system\": \"%.02Lf\","
        "\"idle\": \"%.02Lf\","
        "\"iowait\": \"%.02Lf\","
        "\"irq\": \"%.02Lf\","
        "\"softirq\": \"%.02Lf\","
        "\"steal\": \"%.02Lf\","
        "\"guest\": \"%.02Lf\","
        "\"guestNice\": \"%.02Lf\"}",
        tperc.user, tperc.nice, tperc.system,
        tperc.idle, tperc.iowait, tperc.irq,
        tperc.softirq, tperc.steal,
        tperc.guest, tperc.guest_nice);

    responseSetHeader(res, "content-type", "application/json");
    responseSendBytes(res, buffer, strlen(buffer));
}

void cpuTimes(Request* req, Response* res)
{
    char buffer[256] = { 0 };
    cpu_times_t times;
    cpu_times(&times);

    sprintf(buffer,
        "{\"user\": \"%.0Lf\","
        "\"nice\": \"%.0Lf\","
        "\"system\": \"%.0Lf\","
        "\"idle\": \"%.0Lf\","
        "\"iowait\": \"%.0Lf\","
        "\"irq\": \"%.0Lf\","
        "\"softirq\": \"%.0Lf\","
        "\"steal\": \"%.0Lf\","
        "\"guest\": \"%.0Lf\","
        "\"guestNice\": \"%.0Lf\"}",
        times.user, times.nice, times.system,
        times.idle, times.iowait, times.irq,
        times.softirq, times.steal,
        times.guest, times.guest_nice);

    responseSetHeader(res, "content-type", "application/json");
    responseSendBytes(res, buffer, strlen(buffer));
}

void diskIoCounters(Request* req, Response* res)
{
    sdiskio_t io;
    disk_io_counters(&io, false);
    char buffer[2048] = { 0 };

    sprintf(buffer,
        "{\"readCount\": \"%llu\",\"writeCount\": \"%llu\","
        "\"readBytes\": \"%llu\",\"writeBytes\": \"%llu\","
        "\"readTime\": \"%llu\",\"writeTime\": \"%llu\","
        "\"readMergedCount\": \"%llu\",\"writeMergedCount\": \"%llu\"}",
        io.read_count, io.write_count, io.read_bytes, io.write_bytes,
        io.read_time, io.write_time, io.read_merged_count, io.write_merged_count);

    responseSetHeader(res, "content-type", "application/json");
    responseSendBytes(res, buffer, strlen(buffer));
}

void diskPartitions(Request* req, Response* res)
{
    const int MAX_PARTS = 10;
    sdiskpart_t parts[MAX_PARTS];
    sdiskusage_t usage;
    char buffer[8096] = { 0 };
    char partbuf[512];

    int np = disk_partitions(parts, MAX_PARTS, false);

    sprintf(buffer, "{\"disk\": [%s", (np < 1) ? "]}" : "\0");

    for (int i = 0; i < np; i++) {
        memset(partbuf, 0, sizeof partbuf);
        memset(&usage, 0, sizeof usage);

        disk_usage(&usage, parts[i].mountpoint);

        sprintf(partbuf,
            "{"
            "\"part\":{\"dev\":\"%s\",\"mount\":\"%s\",\"fstype\":\"%s\",\"opts\":\"%s\",\"maxfile\":\"%u\",\"maxpath\":\"%u\"},"
            "\"usage\":{\"total\":\"%llu\",\"used\":\"%llu\",\"free\":\"%llu\",\"percent\":\"%.02f\"}"
            "}%s",
            parts[i].device, parts[i].mountpoint, parts[i].fstype, parts[i].opts, parts[i].maxfile, parts[i].maxpath,
            usage.total, usage.used, usage.free, usage.percent,
            (i + 1) < np ? "," : "]}");

        strcat(buffer, partbuf);
    }

    responseSetHeader(res, "content-type", "application/json");
    responseSendBytes(res, buffer, strlen(buffer));
}

void memoryInfo(Request* req, Response* res)
{
    char buffer[2048] = { 0 };
    memory_info_t mem;

    memory_info(&mem);

    sprintf(buffer,
        "{\"manufacturer\":\"%s\",\"type\":\"%s\",\"formFactor\":\"%s\",\"size\":\"%d\",\"speed\":\"%d\"}",
        mem.manufacturer, mem.type, mem.form_factor, mem.size, mem.speed);

    responseSetHeader(res, "content-type", "application/json");
    responseSendBytes(res, buffer, strlen(buffer));
}

void memoryUsage(Request* req, Response* res)
{
    char buffer[8096] = { 0 };
    svmem_t vmem;
    sswap_t swap;

    virtual_memory(&vmem);
    swap_memory(&swap);

    sprintf(buffer,
        "{\"vmem\":{\"total\":\"%llu\",\"available\":\"%llu\",\"percent\":\"%02.f\",\"used\":\"%llu\","
        "\"free\":\"%llu\",\"active\":\"%llu\",\"inactive\":\"%llu\",\"buffers\":\"%llu\","
        "\"cached\":\"%llu\",\"shared\":\"%llu\",\"slab\":\"%llu\"},"

        "\"swap\": {\"total\":\"%llu\",\"used\":\"%llu\",\"free\":\"%llu\","
        "\"percent\":\"%02.f\",\"sin\":\"%llu\",\"sout\":\"%llu\"}}",
        vmem.total, vmem.available, vmem.percent, vmem.used,
        vmem.free, vmem.active, vmem.inactive, vmem.buffers,
        vmem.cached, vmem.shared, vmem.slab,
        swap.total, swap.used, swap.free,
        swap.percent, swap.sin, swap.sout);

    responseSetHeader(res, "content-type", "application/json");
    responseSendBytes(res, buffer, strlen(buffer));
}

void netIoCounters(Request* req, Response* res)
{
    snetio_t netio = { 0 };
    char buffer[2048] = { 0 };

    net_io_counters(&netio, false);

    sprintf(buffer,
        "{\"interface\":\"%s\","
        "\"bytesSent\":\"%llu\","
        "\"bytesRecv\":\"%llu\","
        "\"packetsSent\":\"%llu\","
        "\"packetsRecv\":\"%llu\","
        "\"errin\":\"%llu\","
        "\"errout\":\"%llu\","
        "\"dropin\":\"%llu\","
        "\"dropout\":\"%llu\"}",
        netio.interface,
        netio.bytes_sent,
        netio.bytes_recv,
        netio.packets_sent,
        netio.packets_recv,
        netio.errin,
        netio.errout,
        netio.dropin,
        netio.dropout);

    responseSetHeader(res, "content-type", "application/json");
    responseSendBytes(res, buffer, strlen(buffer));
}

void batteryInfo(Request* req, Response* res)
{
    ibattery_t ib;
    char buffer[256] = { 0 };

    battery_info(&ib);

    sprintf(buffer,
        "{\"manufacturer\":\"%s\","
        "\"modelName\":\"%s\","
        "\"technology\":\"%s\","
        "\"status\":\"%s\"}",
        ib.manufacturer,
        ib.model_name,
        ib.technology,
        ib.status);

    responseSetHeader(res, "content-type", "application/json");
    responseSendBytes(res, buffer, strlen(buffer));
}

void sensorsBattery(Request* req, Response* res)
{

    char buffer[64] = { 0 };
    sbattery_t sb;

    sensors_battery(&sb);

    sprintf(buffer,
        "{\"present\":\"%s\",\"percent\":\"%u\",\"powerPlugged\":\"%s\"}",
        sb.present ? "true" : "false",
        sb.percent,
        sb.power_plugged ? "true" : "false");

    responseSetHeader(res, "content-type", "application/json");
    responseSendBytes(res, buffer, strlen(buffer));
}

void sensorsTemperatures(Request* req, Response* res)
{
    char buffer[256] = { 0 };
    char sensorbuf[128];
    const int SENSOR_COUNT = 2;
    temp_sensor_t sensor[SENSOR_COUNT];

    sprintf(buffer, "{\"temperatures\":[");

    if (sensors_temperatures(sensor, SENSOR_COUNT) >= 0) {
        for (int i = 0; i < SENSOR_COUNT; i++) {
            sprintf(sensorbuf,
                "{\"name\":\"%s\","
                "\"label\":\"%s\","
                "\"current\":\"%.2f\","
                "\"high\":\"%.2f\","
                "\"critical\":\"%.2f\"}%s",
                sensor[i].name,
                sensor[i].temperatures.label,
                sensor[i].temperatures.current,
                sensor[i].temperatures.high,
                sensor[i].temperatures.critical,
                (i + 1) < SENSOR_COUNT ? "," : "\0");

            strcat(buffer, sensorbuf);
        }
    }
    strcat(buffer, "]}");

    responseSetHeader(res, "content-type", "application/json");
    responseSendBytes(res, buffer, strlen(buffer));
}

void bootTime(Request* req, Response* res)
{
    char buffer[128] = { 0 };
    sprintf(buffer, "{\"btime\": \"%llu\"}", boot_time());
    responseSetHeader(res, "content-type", "application/json");
    responseSendBytes(res, buffer, strlen(buffer));
}

void sysInfo(Request* req, Response* res)
{
    sysinfo_t si;
    sys_info(&si);
    char buffer[2048] = { 0 };

    sprintf(buffer,
        "{\"kernel\": \"%s\","
        "\"release\": \"%s\","
        "\"version\": \"%s\","
        "\"hostname\": \"%s\","
        "\"hwType\": \"%s\","
        "\"chasisVendor\": \"%s\","
        "\"vendor\": \"%s\","
        "\"productName\": \"%s\"}",
        si.kernel, si.release, si.version,
        si.hostname, si.hw_type, si.chasis_vendor,
        si.vendor, si.product_name);

    responseSetHeader(res, "content-type", "application/json");
    responseSendBytes(res, buffer, strlen(buffer));
}

void usershdl(Request* req, Response* res)
{
    suser_t list[10];
    int userCount = users(list, 10);
    char buffer[2048] = { 0 };

    sprintf(buffer, "{\"users\": [");
    if (userCount < 1)
        strcat(buffer, "]}");

    char temp[512];
    for (int i = 0; i < userCount; i++) {
        memset(temp, 0, sizeof temp);
        sprintf(temp,
            "{\"name\": \"%s\","
            "\"terminal\": \"%s\","
            "\"host\": \"%s\","
            "\"started\": \"%llu\","
            "\"pid\": \"%d\"}%s",
            list[i].name, list[i].terminal,
            list[i].host, list[i].started,
            list[i].pid, (i + 1) < userCount ? "," : "");
        strcat(buffer, temp);
    }

    if (userCount >= 1)
        strcat(buffer, "]}");

    responseSetHeader(res, "content-type", "application/json");
    responseSendBytes(res, buffer, strlen(buffer));
}

int main(void)
{
    const void* routingTable[][2] = {
        // CPU
        { "/", indexRedirect },
        { "/cpucount", cpuCount },
        { "/cpufreq", cpuFreq },
        { "/cpuinfo", cpuInfo },
        { "/cpuloadavg", cpuLoadAvg },
        { "/cpupercent", cpuPercent },
        { "/cpustats", cpuStats },
        { "/cputimespercent", cpuTimesPercent },
        { "/cputimes", cpuTimes },
        // DISK
        { "/diskioc", diskIoCounters },
        { "/diskparts", diskPartitions },
        // MEMORY
        { "/meminfo", memoryInfo },
        { "/mu", memoryUsage },
        // NETWORK
        { "/netioc", netIoCounters },
        // SENSORS
        { "/batinfo", batteryInfo },
        { "/sensorbat", sensorsBattery },
        { "/sensortemp", sensorsTemperatures },
        // SYSTEM
        { "/btime", bootTime },
        { "/sysinfo", sysInfo },
        { "/users", usershdl }
    };

    Server server;
    serverInit(&server);
    serverStatic(&server, "src/webclient");

    for (int i = 0; i < ARRAY_SIZE(routingTable); i++)
        serverGet(&server, (char*)routingTable[i][0], (void*)routingTable[i][1]);

    serverListen(&server, "7000");
}
