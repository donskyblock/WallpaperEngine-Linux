export interface Display {
  name: string;
  description: string;
  width: number;
  height: number;
  refresh_rate: number;
  x: number;
  y: number;
  focused: boolean;
}

export interface WorkshopItem {
  id: string;
  title: string;
  preview_url: string;
  description: string;
  tags: string[];
  subscriptions: number;
  favorited: number;
  votes_up: number;
  votes_down: number;
  score: number;
  content_type: string;
  time_updated: number;
  time_created: number;
  file_size: number;
  views: number;
}

export interface WorkshopSearchResult {
  items: WorkshopItem[];
  total: number;
}

export interface AppSettings {
  steam_api_key: string;
  download_dir: string;
  default_monitor: string;
  steam_username: string;
  audio_enabled: boolean;
  fps_limit: number;
}

export interface LibraryItem {
  id: string;
  title: string;
  path: string;
  preview_path: string | null;
  content_type: string;
  workshop_id: string | null;
}

export interface ToolsStatus {
  steamcmd: boolean;
  mpvpaper: boolean;
  awww: boolean;
  hyprpaper: boolean;
  linux_wallpaperengine: boolean;
  we_assets: boolean;
}

export type NavView = 'workshop' | 'library' | 'settings';
export type SortType = 'vote' | 'new' | 'trend' | 'subscriptions';
export type TypeFilter = 'all' | 'Video' | 'Scene' | 'Web';
