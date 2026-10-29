export type Snapshot = {
  version: number;
  date: number;
  data: Uint8Array;
};

export async function saveSnapshot(
  game: string,
  snapshot: Snapshot,
): Promise<void> {
  await transaction("readwrite", (store) => store.put(snapshot, game));
}

export async function loadSnapshot(
  game: string,
): Promise<Snapshot | undefined> {
  return transaction<Snapshot | undefined>("readonly", (store) =>
    store.get(game),
  );
}

const DATABASE = "megado";
const STORE = "snapshots";

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
