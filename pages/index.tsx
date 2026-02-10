import React, { useEffect, useState } from "react";
import ThingMeta from "../components/ThingMeta";
import { useRouter } from "next/router";
import {
  DynamoDBClient
} from "@aws-sdk/client-dynamodb";
import {
  DynamoDBDocumentClient,
  QueryCommand,
  QueryCommandInput
} from "@aws-sdk/lib-dynamodb";


export default function AisleDashboard(): JSX.Element {
  const router = useRouter();
  const navItems: string[] = [
    "Dashboard",
    "Hotspots",
    "Real-Time Map",
    "Historical Data",
    "Settings",
  ];

  const stats: { title: string; value: string; subtitle: string }[] = [
    { title: "Active Carts",      value: "128",   subtitle: "Currently in Store" },
    { title: "Hotspots Detected", value: "42",    subtitle: "This Hour" },
    { title: "Total Visits",      value: "1,254", subtitle: "Today" },
  ];
  const [coordinates, setCoordinates] = useState<any[]>([]);
  const [loading, setLoading] = useState(true);
  useEffect(() => {
  async function fetchCoordinates() {
    setLoading(true);
    try {
      const res = await fetch("/api/coordinates");
      const data = await res.json();
      setCoordinates(Array.isArray(data) ? data : []);
    } catch (err) {
      setCoordinates([]);
    }
    setLoading(false);
  }
  fetchCoordinates();
}, []);

  return (
    <div className="flex h-screen bg-gray-100">
      {/* Sidebar */}
      <aside className="w-64 bg-sidebarBg text-white flex flex-col">
        <div className="p-6 text-2xl font-bold">Aisle Analytics</div>
        <ThingMeta thingName="carttestnode" />
        <nav className="flex-1 mt-6 space-y-1">
    {navItems.map(item =>
      item === "Hotspots" ? (
        <button
          key={item}
          onClick={() => router.push("/heatmap")}
          className="
            block w-full text-left px-6 py-2 rounded
            transition-colors duration-200 ease-in-out
            text-white
            hover:bg-secondary-500/50
            hover:text-white/90
            focus:outline-none
          "
        >
          {item}
        </button>
      ) : (
        <a
          key={item}
          href="#"
          className="
            block px-6 py-2 rounded
            transition-colors duration-200 ease-in-out
            text-white
            hover:bg-secondary-500/50
            hover:text-white/90
          "
        >
          {item}
        </a>
      )
    )}
  </nav>
        
        
      </aside>

      {/* Main Content */}
      <main className="flex-1 overflow-auto p-6">

        {/* Top Stats */}
        <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 gap-6 mb-6">
          {stats.map(s => (
            <div
              key={s.title}
              className="bg-blue-50 border-l-4 border-primary-500 rounded p-4"
            >
              <h3 className="text-lg font-medium text-primary-500">{s.title}</h3>
              <p className="text-2xl mt-2">{s.value}</p>
              <p className="text-sm text-gray-500 mt-1">{s.subtitle}</p>
            </div>
          ))}
        </div>

        {/* Chart & Map Placeholders */}
        <div className="grid grid-cols-1 lg:grid-cols-3 gap-6 mb-6">
          <div className="h-64 p-4 rounded shadow bg-info-500 flex items-center justify-center text-white">
            Real-Time Map
          </div>
          <div className="h-64 p-4 rounded shadow bg-success-500 flex items-center justify-center text-white">
            Hotspot Heatmap
            
          </div>
          <div className="h-64 p-4 rounded shadow bg-warning-500 flex items-center justify-center text-white">
            Daily Trend
          </div>
        </div>

        {/* Tables */}
        <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
          {/* Recent Coordinates */}
          <div className="bg-white shadow rounded p-4">
            <h3 className="text-lg font-medium mb-4">Recent Coordinates</h3>
            <table className="w-full text-left text-sm">
              <thead>
                <tr>
                  <th className="p-2">Device ID</th>
                  <th className="p-2">Timestamp</th>
                  <th className="p-2">RSSI</th>
                </tr>
              </thead>
              <tbody>
                {loading ? (
                  <tr>
                    <td className="p-2" colSpan={3}>Loading...</td>
                  </tr>
                ) : coordinates.length === 0 ? (
                  <tr>
                    <td className="p-2" colSpan={3}>No data</td>
                  </tr>
                ) : (
                  coordinates.map((item, idx) => (
                    <tr key={idx}>
                      <td className="p-2">{item.deviceId}</td>
                      <td className="p-2">{item.timestamp}</td>
                      <td className="p-2">
                        {Array.isArray(item.rssi)
                          ? item.rssi.map((r: any, i: number) => {
                              console.log("RSSI value:", r);
                              return typeof r === "object" && r !== null && "N" in r
                                ? r.N
                                : typeof r === "string" || typeof r === "number"
                                  ? r
                                  : "";
                            }).join(", ")
                          : ""}
                      </td>
                    </tr>
                  ))
                )}
              </tbody>
            </table>
          </div>

          {/* Hotspot Summary */}
          <div className="bg-white shadow rounded p-4">
            <h3 className="text-lg font-medium mb-4">Hotspot Summary</h3>
            <table className="w-full text-left text-sm">
              <thead>
                <tr>
                  <th className="p-2">Zone</th>
                  <th className="p-2">Cart Count</th>
                  <th className="p-2">Peak Time</th>
                </tr>
              </thead>
              <tbody>
                <tr>
                  <td className="p-2">Produce</td><td className="p-2">18</td><td className="p-2">14:00</td>
                </tr>
                <tr>
                  <td className="p-2">Dairy</td><td className="p-2">12</td><td className="p-2">13:45</td>
                </tr>
              </tbody>
            </table>
          </div>
        </div>
      </main>
    </div>
  );
}
