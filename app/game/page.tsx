"use client";
import React, { useEffect, useRef, useState } from "react";
import Link from "next/link";
import {
  Note,
  GameState,
  KEYS,
  createNote,
  updateNotes,
  hitNote,
  formatTime,
  getComboText,
} from "@/lib/gamelogic";

export default function GamePage() {
  const laneRefs = useRef<(HTMLDivElement | null)[]>([]);
  const scoreRef = useRef<HTMLDivElement | null>(null);
  const comboRef = useRef<HTMLDivElement | null>(null);
  const timerRef = useRef<HTMLDivElement | null>(null);

  const [gameOver, setGameOver] = useState(false);
  const [finalScore, setFinalScore] = useState(0);
  const [highestCombo, setHighestCombo] = useState(0);
  const [timeLeft, setTimeLeft] = useState(60);

  const restartGame = () => {
    setGameOver(false);
    setFinalScore(0);
    setHighestCombo(0);
    setTimeLeft(60);
  };

  useEffect(() => {
    if (gameOver) return;

    const lanes = laneRefs.current;
    let notes: Note[] = [];
    const gameState: GameState = {
      score: 0,
      combo: 0,
      maxCombo: 0,
      secondsLeft: 60,
    };

    function updateComboDisplay() {
      if (!comboRef.current) return;
      comboRef.current.textContent = getComboText(gameState.combo);
    }

    function updateTimerDisplay() {
      if (!timerRef.current) return;
      timerRef.current.textContent = formatTime(gameState.secondsLeft);
    }

    function updateScoreDisplay(score: number) {
      if (scoreRef.current) {
        scoreRef.current.textContent = score.toString();
      }
    }

    function spawnNote() {
      const laneIndex = Math.floor(Math.random() * 4);
      const lane = lanes[laneIndex];
      if (!lane) return;

      const note = createNote(laneIndex, lane);
      if (note) {
        notes.push(note);
      }
    }

    function update() {
      notes = updateNotes(notes, gameState, updateComboDisplay);
    }

    function hit(laneIndex: number) {
      notes = hitNote(
        laneIndex,
        notes,
        gameState,
        updateScoreDisplay,
        updateComboDisplay
      );
    }

    const handleKeyDown = (e: KeyboardEvent) => {
      if (KEYS[e.key] !== undefined) hit(KEYS[e.key]);
    };

    document.addEventListener("keydown", handleKeyDown);

    const spawnInterval = setInterval(spawnNote, 700);
    const updateInterval = setInterval(update, 16);

    const timerInterval = setInterval(() => {
      gameState.secondsLeft--;
      setTimeLeft(gameState.secondsLeft);
      updateTimerDisplay();

      if (gameState.secondsLeft <= 0) {
        setFinalScore(gameState.score);
        setHighestCombo(gameState.maxCombo);
        setGameOver(true);
      }
    }, 1000);

    updateTimerDisplay();

    return () => {
      document.removeEventListener("keydown", handleKeyDown);
      clearInterval(spawnInterval);
      clearInterval(updateInterval);
      clearInterval(timerInterval);
      
      // Clean up all notes when unmounting
      notes.forEach(note => {
        if (note.el && note.el.parentNode) {
          note.el.remove();
        }
      });
    };
  }, [gameOver]);

  return (
    <>
      <style>{`
        body {
          background: #111;
          color: white;
          display: flex;
          justify-content: center;
          align-items: center;
          height: 100vh;
          margin: 0;
          font-family: Arial, sans-serif;
        }
        #game {
          width: 600px;
          height: 800px;
          background: #222;
          position: relative;
          overflow: hidden;
          border: 3px solid white;
          display: flex;
        }
        .lane {
          width: 25%;
          height: 100%;
          border-left: 1px solid #444;
          border-right: 1px solid #444;
          position: relative;
        }
        .note {
          width: 100%;
          height: 50px;
          background: cyan;
          position: absolute;
          top: -50px;
          border-radius: 8px;
          display: flex;
          align-items: center;
          justify-content: center;
          font-size: 32px;
          font-weight: bold;
          color: #000;
        }
        .note[data-lane="0"]::before { content: '←'; }
        .note[data-lane="1"]::before { content: '↓'; }
        .note[data-lane="2"]::before { content: '↑'; }
        .note[data-lane="3"]::before { content: '→'; }

        .note.hit {
          background: lime;
        }

        #hitbar {
          position: absolute;
          bottom: 0;
          width: 100%;
          height: 60px;
          background: rgba(255,255,255,0.2);
          pointer-events: none;
        }

        #score {
          position: absolute;
          top: 20px;
          right: 20px;
          font-size: 36px;
          font-weight: bold;
          z-index: 10;
        }

        #combo {
          position: absolute;
          top: 65px;
          right: 20px;
          font-size: 28px;
          font-weight: bold;
          color: yellow;
          z-index: 10;
        }

        #timer {
          position: absolute;
          top: 20px;
          left: 20px;
          font-size: 36px;
          font-weight: bold;
          color: ${timeLeft <= 10 ? "red" : "white"};
          z-index: 10;
        }

        .game-over-overlay {
          position: absolute;
          top: 0;
          left: 0;
          width: 100%;
          height: 100%;
          background: rgba(0, 0, 0, 0.85);
          display: flex;
          justify-content: center;
          align-items: center;
          z-index: 1000;
        }

        .game-over-box {
          background: rgba(30, 30, 30, 0.95);
          border: 3px solid white;
          border-radius: 20px;
          padding: 40px 60px;
          text-align: center;
          box-shadow: 0 0 50px rgba(255, 255, 255, 0.3);
        }
      `}</style>

      <div id="game">
        <div id="timer" ref={timerRef}>
          1:00
        </div>
        <div id="score" ref={scoreRef}>
          0
        </div>
        <div id="combo" ref={comboRef}></div>

        {[0, 1, 2, 3].map((i) => (
          <div
            key={i}
            className="lane"
            ref={(el) => {
              laneRefs.current[i] = el;
            }}
          ></div>
        ))}

        <div id="hitbar"></div>

        {gameOver && (
          <div className="game-over-overlay">
            <div className="game-over-box">
              <h1 className="text-5xl font-bold mb-4 text-white">Game Over!</h1>
              <p className="text-3xl mb-4 text-white">
                Final Score:{" "}
                <span className="text-yellow-400 font-bold">{finalScore}</span>
              </p>
              <p className="text-2xl mb-8 text-white">
                Highest Combo:{" "}
                <span className="text-orange-400 font-bold">
                  {highestCombo}x
                </span>
              </p>

              <div className="flex gap-4 justify-center">
                <button
                  onClick={restartGame}
                  className="px-8 py-4 bg-blue-600 hover:bg-blue-700 rounded-xl text-xl font-semibold transition-all text-white"
                >
                  Play Again
                </button>

                <Link
                  href="/"
                  className="px-8 py-4 bg-green-600 hover:bg-green-700 rounded-xl text-xl font-semibold transition-all text-white inline-block"
                >
                  Main Menu
                </Link>
              </div>
            </div>
          </div>
        )}
      </div>
    </>
  );
}