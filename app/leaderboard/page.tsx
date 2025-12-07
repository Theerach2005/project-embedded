"use client";

import React, { useState, useEffect } from "react";
import {
  db,
  collection,
  query,
  orderBy,
  limit,
  getDocs
} from "@/lib/firebase";
import { Timestamp } from "firebase/firestore";

// Define the interface for data coming from Firestore
interface FirestoreScoreData {
  score: number;
  highestCombo: number;
  timestamp: Timestamp;
}

// Define the interface stored in React state
interface ScoreEntry {
  score: number;
  highestCombo: number;
  timestamp: string; // ISO string
}

export default function LeaderboardPage() {
  const [scores, setScores] = useState<ScoreEntry[]>([]);
  const [isLoaded, setIsLoaded] = useState(false);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    setIsLoaded(false);
    setError(null);

    async function fetchScores() {
      try {
        const scoresCollectionRef = collection(db, "highscores");

        const q = query(
          scoresCollectionRef,
          orderBy("score", "desc"),
          orderBy("timestamp", "asc"),
          limit(100)
        );

        const querySnapshot = await getDocs(q);

        const leaderboardData: ScoreEntry[] = [];

        querySnapshot.forEach((doc) => {
          const data = doc.data() as FirestoreScoreData;

          const timestampString =
            data.timestamp?.toDate?.().toISOString() ?? new Date().toISOString();

          leaderboardData.push({
            score: data.score,
            highestCombo: data.highestCombo,
            timestamp: timestampString
          });
        });

        setScores(leaderboardData);
        setError(null);
      } catch (e) {
        console.error("Error fetching scores:", e);
        setError("Failed to load leaderboard data. Check Firebase config/rules.");
      } finally {
        setIsLoaded(true);
      }
    }

    fetchScores();
  }, []);

  const getPlayerName = (index: number): string => {
    switch (index) {
      case 0:
        return "🏆 First Place";
      case 1:
        return "⭐ Second Place";
      case 2:
        return "✨ Third Place";
      default:
        return `Anon Player #${index + 1}`;
    }
  };

  return (
    <div className="min-h-screen bg-[#0f0f0f] text-white flex items-center justify-center p-4 sm:p-8 font-inter">
      <div className="w-full max-w-2xl bg-[#1a1a1a] rounded-2xl p-6 sm:p-8 shadow-2xl border border-white/10">

        <h1 className="text-3xl sm:text-4xl font-extrabold text-center mb-8 bg-clip-text text-transparent bg-gradient-to-r from-purple-400 to-pink-600">
          🎵 Global Leaderboard
        </h1>

        <div className="overflow-x-auto rounded-xl shadow-lg border border-white/10">
          <table className="min-w-full table-auto">
            <thead className="bg-purple-600/30 text-lg border-b border-white/10">
              <tr>
                <th className="p-3 w-16 sm:w-20 text-left pl-6">#</th>
                <th className="p-3 text-left">Top Score</th>
                <th className="p-3 w-28 sm:w-32 text-center">Score</th>
                <th className="p-3 w-28 sm:w-32 text-center pr-6">Max Combo</th>
              </tr>
            </thead>

            <tbody>
              {error ? (
                <tr>
                  <td colSpan={4} className="p-4 text-center text-red-400">
                    Error: {error}
                  </td>
                </tr>
              ) : !isLoaded ? (
                <tr>
                  <td
                    colSpan={4}
                    className="p-4 text-center text-white/50 animate-pulse"
                  >
                    Loading Leaderboard Data...
                  </td>
                </tr>
              ) : (
                scores.map((entry, index) => (
                  <tr
                    key={index}
                    className="odd:bg-white/5 even:bg-white/0 hover:bg-white/10 transition-all duration-200 border-b border-white/5 last:border-b-0"
                  >
                    <td
                      className="p-3 text-left pl-6 font-extrabold"
                      style={{ color: index < 3 ? "gold" : "inherit" }}
                    >
                      {index === 0 && "🥇"}
                      {index === 1 && "🥈"}
                      {index === 2 && "🥉"}
                      {index >= 3 && `#${index + 1}`}
                    </td>

                    <td className="p-3 text-left">
                      <span className="truncate block max-w-[150px]">
                        {getPlayerName(index)}
                      </span>
                    </td>

                    <td className="p-3 text-center font-semibold text-pink-400">
                      {entry.score.toLocaleString()}
                    </td>

                    <td className="p-3 text-center text-blue-300 font-semibold pr-6">
                      x{entry.highestCombo}
                    </td>
                  </tr>
                ))
              )}

              {isLoaded && scores.length === 0 && !error && (
                <tr>
                  <td colSpan={4} className="p-4 text-center text-white/50">
                    No scores yet! Be the first to play.
                  </td>
                </tr>
              )}
            </tbody>
          </table>
        </div>

        <div className="text-center mt-8">
          <a
            href="/"
            className="inline-flex items-center space-x-2 px-6 py-3 bg-purple-600 hover:bg-purple-700 rounded-full text-white font-bold shadow-lg transition duration-300 transform hover:scale-[1.02]"
          >
            <svg
              xmlns="http://www.w3.org/2000/svg"
              className="h-5 w-5"
              viewBox="0 0 20 20"
              fill="currentColor"
            >
              <path
                fillRule="evenodd"
                d="M9.707 14.707a1 1 0 01-1.414 0l-4-4a1 1 0 010-1.414l4-4a1 1 0 011.414 1.414L7.414 9H15a1 1 0 110 2H7.414l2.293 2.293a1 1 0 010 1.414z"
                clipRule="evenodd"
              />
            </svg>
            <span>Back to MainPage</span>
          </a>
        </div>
      </div>
    </div>
  );
}
