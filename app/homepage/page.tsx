import Link from "next/link";

export default function HomePage() {
  return (
    <main className="min-h-screen bg-[#0f0f0f] text-white flex items-center justify-center p-8 font-inter">
      <div className="w-full max-w-xl bg-[#1a1a1a] rounded-2xl p-8 shadow-2xl border border-white/10 text-center">

        <h1 className="text-4xl font-extrabold mb-8 bg-clip-text text-transparent bg-gradient-to-r from-purple-400 to-pink-600">
          🎮 Rhythm Game
        </h1>

        <div className="flex flex-col gap-6 mt-4">

          <Link
            href="/game"
            className="inline-flex justify-center items-center px-6 py-3 bg-purple-600 hover:bg-purple-700 focus:bg-purple-700 focus:outline-none focus:ring-4 focus:ring-purple-500/50 rounded-full text-white font-bold shadow-lg transition duration-300 transform hover:scale-[1.03] focus:scale-[1.03]"
          >
            Play Game
          </Link>

          <Link
            href="/leaderboard"
            className="inline-flex justify-center items-center px-6 py-3 bg-pink-600 hover:bg-pink-700 focus:bg-pink-700 focus:outline-none focus:ring-4 focus:ring-pink-500/50 rounded-full text-white font-bold shadow-lg transition duration-300 transform hover:scale-[1.03] focus:scale-[1.03]"
          >
            View Leaderboard
          </Link>

        </div>
      </div>
    </main>
  );
}