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

import styles from './game.module.css';

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
    // Reset state variables
    setGameOver(false);
    setFinalScore(0);
    setHighestCombo(0);
    setTimeLeft(60);

    // Manually reset the display text content in the DOM via refs
    if (scoreRef.current) {
      scoreRef.current.textContent = "0";
    }
    if (comboRef.current) {
      comboRef.current.textContent = "";
    }
    if (timerRef.current) {
      timerRef.current.textContent = formatTime(60);
    }
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

    // Initialize displays immediately when the game starts/restarts
    updateScoreDisplay(gameState.score);
    updateComboDisplay();
    updateTimerDisplay();

    function spawnNote() {
      const laneIndex = Math.floor(Math.random() * 4);
      const lane = lanes[laneIndex];
      if (!lane) return;

      const note = createNote(laneIndex, lane, styles.note); 
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
        updateComboDisplay,
        styles.noteHit
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
      <div className={styles.body}>
        <div className={styles.game}>
          <div
            className={`${styles.timer} ${timeLeft <= 10 ? styles.timerWarning : styles.timerNormal}`}
            ref={timerRef}
          >
            {formatTime(timeLeft)}
          </div>
          <div className={styles.score} ref={scoreRef}>
            0
          </div>
          <div className={styles.combo} ref={comboRef}></div>

          {[0, 1, 2, 3].map((i) => (
            <div
              key={i}
              className={styles.lane}
              ref={(el) => {
                laneRefs.current[i] = el;
              }}
            ></div>
          ))}

          <div className={styles.hitbar}></div>

          {gameOver && (
            <div className={styles.gameOverOverlay}>
              <div className={styles.gameOverBox}>
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
      </div>
    </>
  );
}