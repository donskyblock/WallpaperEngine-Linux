import { create } from 'zustand';
import { invoke } from '@tauri-apps/api/core';
import type {
  AppSettings,
  Display,
  LibraryItem,
  NavView,
  SortType,
  ToolsStatus,
  TypeFilter,
  WorkshopItem,
} from '../types';

interface AppState {
  // nav
  view: NavView;
  setView: (v: NavView) => void;

  // displays
  displays: Display[];
  selectedDisplay: string;
  loadDisplays: () => Promise<void>;
  setSelectedDisplay: (name: string) => void;

  // workshop
  workshopItems: WorkshopItem[];
  workshopTotal: number;
  workshopPage: number;
  workshopSearch: string;
  workshopSort: SortType;
  workshopTypeFilter: TypeFilter;
  isLoadingWorkshop: boolean;
  workshopError: string | null;
  selectedItem: WorkshopItem | null;
  setSelectedItem: (item: WorkshopItem | null) => void;
  searchWorkshop: (query?: string) => Promise<void>;
  loadNextPage: () => Promise<void>;
  setSort: (sort: SortType) => Promise<void>;
  setTypeFilter: (filter: TypeFilter) => Promise<void>;

  // library
  libraryItems: LibraryItem[];
  isLoadingLibrary: boolean;
  loadLibrary: () => Promise<void>;

  // wallpaper actions
  isSettingWallpaper: boolean;
  applyWallpaper: (path: string, contentType: string) => Promise<void>;
  applyLibraryItem: (item: LibraryItem) => Promise<void>;
  stopWallpaper: () => Promise<void>;

  // download
  downloadingIds: Set<string>;
  downloadItem: (item: WorkshopItem) => Promise<void>;
  toolsStatus: ToolsStatus;
  checkTools: () => Promise<void>;

  // settings
  settings: AppSettings;
  loadSettings: () => Promise<void>;
  saveSettings: (s: Partial<AppSettings>) => Promise<void>;

  // toast
  toast: { message: string; type: 'success' | 'error' | 'info' } | null;
  showToast: (message: string, type?: 'success' | 'error' | 'info') => void;
}

const DEFAULT_SETTINGS: AppSettings = {
  steam_api_key: '',
  download_dir: '',
  default_monitor: '',
  steam_username: '',
  audio_enabled: true,
  fps_limit: 30,
};

export const useAppStore = create<AppState>((set, get) => ({
  view: 'workshop',
  setView: (view) => set({ view }),

  displays: [],
  selectedDisplay: '',
  loadDisplays: async () => {
    try {
      const displays = await invoke<Display[]>('get_displays');
      const focused = displays.find((d) => d.focused);
      const { settings } = get();
      set({
        displays,
        selectedDisplay:
          settings.default_monitor ||
          focused?.name ||
          displays[0]?.name ||
          '',
      });
    } catch (e) {
      console.error('Failed to load displays:', e);
    }
  },
  setSelectedDisplay: (name) => set({ selectedDisplay: name }),

  workshopItems: [],
  workshopTotal: 0,
  workshopPage: 1,
  workshopSearch: '',
  workshopSort: 'vote',
  workshopTypeFilter: 'all',
  isLoadingWorkshop: false,
  workshopError: null,
  selectedItem: null,
  setSelectedItem: (item) => set({ selectedItem: item }),

  searchWorkshop: async (query) => {
    const { settings, workshopSort, workshopTypeFilter, showToast } = get();
    const q = query !== undefined ? query : get().workshopSearch;
    set({ isLoadingWorkshop: true, workshopError: null, workshopPage: 1, workshopSearch: q });
    try {
      const result = await invoke<{ items: WorkshopItem[]; total: number }>(
        'search_workshop',
        {
          apiKey: settings.steam_api_key,
          query: q,
          page: 1,
          sort: workshopSort,
          typeFilter: workshopTypeFilter === 'all' ? '' : workshopTypeFilter,
        }
      );
      set({ workshopItems: result.items, workshopTotal: result.total });
    } catch (e: unknown) {
      const msg = e instanceof Error ? e.message : String(e);
      set({ workshopError: msg });
      showToast(msg, 'error');
    } finally {
      set({ isLoadingWorkshop: false });
    }
  },

  loadNextPage: async () => {
    const { settings, workshopSort, workshopTypeFilter, workshopSearch, workshopPage, workshopItems, isLoadingWorkshop } = get();
    if (isLoadingWorkshop) return;
    set({ isLoadingWorkshop: true });
    try {
      const nextPage = workshopPage + 1;
      const result = await invoke<{ items: WorkshopItem[]; total: number }>(
        'search_workshop',
        {
          apiKey: settings.steam_api_key,
          query: workshopSearch,
          page: nextPage,
          sort: workshopSort,
          typeFilter: workshopTypeFilter === 'all' ? '' : workshopTypeFilter,
        }
      );
      set({
        workshopItems: [...workshopItems, ...result.items],
        workshopPage: nextPage,
      });
    } catch (e) {
      console.error(e);
    } finally {
      set({ isLoadingWorkshop: false });
    }
  },

  setSort: async (sort) => {
    set({ workshopSort: sort });
    await get().searchWorkshop();
  },

  setTypeFilter: async (filter) => {
    set({ workshopTypeFilter: filter });
    await get().searchWorkshop();
  },

  libraryItems: [],
  isLoadingLibrary: false,
  loadLibrary: async () => {
    const { settings } = get();
    set({ isLoadingLibrary: true });
    try {
      const items = await invoke<LibraryItem[]>('scan_library', {
        downloadDir: settings.download_dir,
      });
      set({ libraryItems: items });
    } catch (e) {
      console.error('Failed to load library:', e);
    } finally {
      set({ isLoadingLibrary: false });
    }
  },

  isSettingWallpaper: false,
  applyWallpaper: async (path, contentType) => {
    const { selectedDisplay, showToast } = get();
    set({ isSettingWallpaper: true });
    try {
      await invoke('set_wallpaper', {
        path,
        monitor: selectedDisplay,
        contentType,
      });
      showToast('Wallpaper applied!', 'success');
    } catch (e: unknown) {
      const msg = e instanceof Error ? e.message : String(e);
      showToast(msg, 'error');
    } finally {
      set({ isSettingWallpaper: false });
    }
  },
  applyLibraryItem: async (item) => {
    await get().applyWallpaper(item.path, item.content_type);
  },
  stopWallpaper: async () => {
    const { selectedDisplay } = get();
    await invoke('stop_wallpaper', { monitor: selectedDisplay });
  },

  downloadingIds: new Set(),
  downloadItem: async (item) => {
    const { settings, showToast, downloadingIds } = get();
    if (downloadingIds.has(item.id)) return;
    set({ downloadingIds: new Set([...downloadingIds, item.id]) });
    try {
      const path = await invoke<string>('download_workshop_item', {
        itemId: item.id,
        steamUsername: settings.steam_username,
        downloadDir: settings.download_dir,
      });
      showToast(`Downloaded: ${item.title}`, 'success');
      await get().loadLibrary();
      return path as unknown as void;
    } catch (e: unknown) {
      const msg = e instanceof Error ? e.message : String(e);
      showToast(msg, 'error');
    } finally {
      const ids = new Set(get().downloadingIds);
      ids.delete(item.id);
      set({ downloadingIds: ids });
    }
  },

  toolsStatus: { steamcmd: false, mpvpaper: false, awww: false, hyprpaper: false, linux_wallpaperengine: false, we_assets: false },
  checkTools: async () => {
    const status = await invoke<ToolsStatus>('check_tools');
    set({ toolsStatus: status });
  },

  settings: DEFAULT_SETTINGS,
  loadSettings: async () => {
    try {
      const settings = await invoke<AppSettings>('load_settings');
      set({ settings });
    } catch (e) {
      console.error('Failed to load settings:', e);
    }
  },
  saveSettings: async (partial) => {
    const { settings, showToast } = get();
    const updated = { ...settings, ...partial };
    try {
      await invoke('save_settings', { settings: updated });
      set({ settings: updated });
      showToast('Settings saved', 'success');
    } catch (e: unknown) {
      const msg = e instanceof Error ? e.message : String(e);
      showToast(msg, 'error');
    }
  },

  toast: null,
  showToast: (message, type = 'info') => {
    set({ toast: { message, type } });
    setTimeout(() => set({ toast: null }), 3500);
  },
}));
