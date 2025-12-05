import Link from "next/link";

export default function HomePage() {
  return (
    <main className="min-h-screen flex flex-col items-center justify-center gap-6 p-8 bg-gray-900 text-white">
      <h1 className="text-4xl font-bold mb-6">Welcome to the Game</h1>

      <Link
        href="/game"
        className="px-6 py-3 bg-blue-600 hover:bg-blue-700 rounded-xl text-lg font-semibold transition-all"
      >
        Play Game
      </Link>

      <Link
        href="/leaderboard"
        className="px-6 py-3 bg-green-600 hover:bg-green-700 rounded-xl text-lg font-semibold transition-all"
      >
        View Leaderboard
      </Link>
    </main>
  );
}