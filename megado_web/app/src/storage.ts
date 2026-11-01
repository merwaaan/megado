export type SaveState = {
  version: number;
  date: number;
  data: Uint8Array;
};

export async function saveState(game: string, state: SaveState): Promise<void> {
  await transaction("readwrite", (store) => store.put(state, game));
}

export async function loadState(game: string): Promise<SaveState | undefined> {
  return transaction<SaveState | undefined>("readonly", (store) =>
    store.get(game),
  );
}

const DATABASE = "megado";
const STORE = "states";

function open(): Promise<IDBDatabase> {
  return new Promise((resolve, reject) => {
    const request = indexedDB.open(DATABASE, 1);

    request.onupgradeneeded = () => request.result.createObjectStore(STORE);
    request.onsuccess = () => resolve(request.result);
    request.onerror = () => reject(request.error);
  });
}

async function transaction<T>(
  mode: IDBTransactionMode,
  action: (store: IDBObjectStore) => IDBRequest<T>,
): Promise<T> {
  const database = await open();

  try {
    return await new Promise<T>((resolve, reject) => {
      const request = action(
        database.transaction(STORE, mode).objectStore(STORE),
      );

      request.onsuccess = () => resolve(request.result);
      request.onerror = () => reject(request.error);
    });
  } finally {
    database.close();
  }
}
