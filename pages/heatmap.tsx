import React from "react";
import { useRouter } from "next/router";

export default function HeatmapPage() {
  const router = useRouter();

  const hotspots = [
    { x: 60, y: 80, intensity: 0.9 },
    { x: 200, y: 120, intensity: 0.7 },
    { x: 320, y: 200, intensity: 0.5 },
    { x: 150, y: 250, intensity: 0.8 },
  ];

  return (
    <div className="flex h-screen bg-gray-100">
      {/* Sidebar */}
      <aside className="w-64 bg-sidebarBg text-white flex flex-col">
        <div className="p-6 text-2xl font-bold">Aisle Analytics</div>
        <nav className="flex-1 mt-6 space-y-1">
          <button
            onClick={() => router.push("/")}
            className="
              block w-full text-left px-6 py-2 rounded
              transition-colors duration-200 ease-in-out
              text-white
              hover:bg-secondary-500/50
              hover:text-white/90
              focus:outline-none
            "
          >
            ← Back to Dashboard
          </button>
        </nav>
      </aside>

      {/* Main Content */}
      <main className="flex-1 overflow-auto p-8">
        <h1 className="text-2xl font-bold mb-4">Heatmap</h1>
        <div className="w-full max-w-xl h-96 bg-gray-200 flex items-center justify-center rounded relative overflow-hidden mx-auto">
          {/* Dummy hotspots */}
          {hotspots.map((spot, idx) => (
            <div
              key={idx}
              className="absolute rounded-full pointer-events-none"
              style={{
                left: spot.x,
                top: spot.y,
                width: 60,
                height: 60,
                background: `rgba(239,68,68,${spot.intensity})`,
                boxShadow: `0 0 40px 20px rgba(239,68,68,${spot.intensity * 0.7})`,
                transform: "translate(-50%, -50%)",
              }}
            />
          ))}
          <span className="text-gray-500 absolute bottom-2 right-4 text-xs">Demo Hotspots</span>
        </div>
      </main>
    </div>
  );
}