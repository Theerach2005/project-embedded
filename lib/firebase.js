import { initializeApp } from "firebase/app";
import { 
  getDatabase, 
  ref, 
  push, 
  set, 
  query, 
  orderByChild, 
  limitToLast, 
  get,
  onValue,
  off,
  remove,
  update
} from "firebase/database";

const firebaseConfig = {
  apiKey: process.env.NEXT_PUBLIC_FIREBASE_API_KEY,
  authDomain: process.env.NEXT_PUBLIC_FIREBASE_AUTH_DOMAIN,
  databaseURL: process.env.NEXT_PUBLIC_FIREBASE_DATABASE_URL,
  projectId: process.env.NEXT_PUBLIC_FIREBASE_PROJECT_ID,
  storageBucket: process.env.NEXT_PUBLIC_FIREBASE_STORAGE_BUCKET,
  messagingSenderId: process.env.NEXT_PUBLIC_FIREBASE_MESSAGING_SENDER_ID,
  appId: process.env.NEXT_PUBLIC_FIREBASE_APP_ID,
};

// Initialize Firebase App
const app = initializeApp(firebaseConfig);

// Initialize Realtime Database
export const database = getDatabase(app);

// Export all Realtime Database functions
export { 
  ref,           // Create database reference
  push,          // Generate unique key and add data
  set,           // Write data to specific location
  query,         // Create database query
  orderByChild,  // Order query results by child key
  limitToLast,   // Limit query to last N items
  get,           // Get data once
  onValue,       // Listen for real-time updates
  off,           // Stop listening for updates
  remove,        // Delete data
  update         // Update specific fields
};