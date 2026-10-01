import { useEffect, useRef, useState } from 'react';
import { Download, Play, X, Star, Eye, Heart, Tag } from 'lucide-react';
import { listen } from '@tauri-apps/api/event';
import { useAppStore } from '../../store/app-store';

export default function WallpaperDetail() {
  const { selectedItem, setSelectedItem, applyWallpaper, downloadItem, downloadingIds, toolsStatus, isSettingWallpaper, libraryItems } = useAppStore();
  const hasSteamcmd = toolsStatus.steamcmd;
  const [downloadLines, setDownloadLines] = useState<string[]>([]);
  const logRef = useRef<HTMLDivElement>(null);

  // Derive before early return so hooks are always called unconditionally
  const item = selectedItem;
  const isDownloading = item ? downloadingIds.has(item.id) : false;
  const isInLibrary = item ? libraryItems.some((l) => l.workshop_id === item.id) : false;
  const localItem = item ? libraryItems.find((l) => l.workshop_id === item.id) : undefined;

  // Subscribe to live steamcmd output while this item is downloading
  useEffect(() => {
    if (!isDownloading || !item) return;
    setDownloadLines([]);
    let unlisten: (() => void) | undefined;
    listen<{ item_id: string; line: string }>('download-progress', (ev) => {
      if (ev.payload.item_id === item.id) {
        setDownloadLines((prev) => [...prev, ev.payload.line]);
      }
    }).then((fn) => { unlisten = fn; });
    return () => { unlisten?.(); };
  }, [isDownloading, item?.id]);

  // Auto-scroll log to bottom on new lines
  useEffect(() => {
    if (logRef.current) logRef.current.scrollTop = logRef.current.scrollHeight;
  }, [downloadLines]);

  if (!item) return null;

  const subs = item.subscriptions > 999
    ? `${(item.subscriptions / 1000).toFixed(1)}k`
    : String(item.subscriptions);

  const score = Math.round(item.score * 100);
  const type = item.content_type.toLowerCase();

  const handleApply = async () => {
    if (localItem) {
      await applyWallpaper(localItem.path, localItem.content_type);
    }
  };

  const handleDownload = async () => {
    await downloadItem(item);
  };

  return (
    <div
      style={{
        width: 280,
        minWidth: 280,
        background: 'var(--bg-sidebar)',
        borderLeft: '1px solid var(--border)',
        display: 'flex',
        flexDirection: 'column',
        overflow: 'hidden',
      }}
    >
      <div style={{ position: 'relative' }}>
        <img
          src={item.preview_url}
          alt={item.title}
          style={{ width: '100%', aspectRatio: '16/9', objectFit: 'cover', display: 'block' }}
        />
        <button
          onClick={() => setSelectedItem(null)}
          style={{
            position: 'absolute',
            top: 8,
            right: 8,
            background: 'rgba(0,0,0,0.6)',
            border: 'none',
            borderRadius: '50%',
            width: 24,
            height: 24,
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'center',
            cursor: 'pointer',
            color: '#fff',
          }}
        >
          <X size={12} />
        </button>
      </div>

      <div style={{ padding: '12px 14px', flex: 1, overflowY: 'auto' }}>
        <h2 style={{ fontSize: 13, fontWeight: 600, color: 'var(--text-primary)', marginBottom: 8, lineHeight: 1.3 }}>
          {item.title}
        </h2>

        <div style={{ display: 'flex', gap: 12, marginBottom: 10 }}>
          <Stat icon={<Star size={11} fill="#f0c040" color="#f0c040" />} value={`${score}%`} />
          <Stat icon={<Heart size={11} color="#f87171" />} value={subs} />
          <Stat icon={<Eye size={11} color="var(--text-muted)" />} value={fmt(item.views)} />
        </div>

        {item.description && (
          <p style={{ fontSize: 11, color: 'var(--text-secondary)', lineHeight: 1.5, marginBottom: 10 }}>
            {item.description.slice(0, 200)}{item.description.length > 200 ? '…' : ''}
          </p>
        )}

        <div style={{ display: 'flex', flexWrap: 'wrap', gap: 4, marginBottom: 12 }}>
          {item.tags.slice(0, 8).map((tag) => (
            <span key={tag} style={{
              display: 'flex',
              alignItems: 'center',
              gap: 3,
              fontSize: 10,
              color: 'var(--text-secondary)',
              background: 'var(--bg-card)',
              padding: '2px 6px',
              borderRadius: 3,
              border: '1px solid var(--border)',
            }}>
              <Tag size={8} />
              {tag}
            </span>
          ))}
        </div>

        <div style={{ fontSize: 11, color: 'var(--text-muted)' }}>
          Type: <span style={{ color: 'var(--text-secondary)' }}>{item.content_type}</span>
          {item.file_size > 0 && (
            <> · {(item.file_size / 1024 / 1024).toFixed(1)} MB</>
          )}
        </div>
      </div>

      <div style={{ padding: '12px 14px', borderTop: '1px solid var(--border)', display: 'flex', flexDirection: 'column', gap: 8 }}>
        {isInLibrary ? (
          <button
            onClick={handleApply}
            disabled={isSettingWallpaper}
            style={btnStyle('var(--accent)', isSettingWallpaper)}
          >
            <Play size={13} />
            {isSettingWallpaper ? 'Applying…' : 'Apply Wallpaper'}
          </button>
        ) : (
          <>
            {!hasSteamcmd && (
              <div style={{ fontSize: 10, color: 'var(--warning)', background: 'rgba(255,152,0,0.1)', padding: '6px 8px', borderRadius: 4 }}>
                Install steamcmd to download: <code style={{ fontSize: 9 }}>yay -S steamcmd</code>
              </div>
            )}
            <button
              onClick={handleDownload}
              disabled={isDownloading || !hasSteamcmd}
              style={btnStyle('#2a2a2a', isDownloading || !hasSteamcmd)}
            >
              <Download size={13} />
              {isDownloading ? 'Downloading…' : 'Download'}
            </button>
            {isDownloading && downloadLines.length > 0 && (
              <div
                ref={logRef}
                style={{
                  maxHeight: 120,
                  overflowY: 'auto',
                  background: 'var(--bg-app)',
                  border: '1px solid var(--border)',
                  borderRadius: 4,
                  padding: '6px 8px',
                  fontFamily: 'monospace',
                  fontSize: 10,
                  color: 'var(--text-muted)',
                  lineHeight: 1.5,
                }}
              >
                {downloadLines.map((line, i) => (
                  <div key={i} style={{ color: line.toLowerCase().includes('error') ? 'var(--warning)' : undefined }}>
                    {line}
                  </div>
                ))}
              </div>
            )}
          </>
        )}

        <div style={{ fontSize: 10, color: 'var(--text-muted)', textAlign: 'center' }}>
          {type === 'video' ? 'Requires mpvpaper for video' :
           type === 'scene' ? 'Requires linux-wallpaperengine' :
           type === 'web' ? 'Web wallpapers coming soon' : ''}
        </div>
      </div>
    </div>
  );
}

function Stat({ icon, value }: { icon: React.ReactNode; value: string }) {
  return (
    <div style={{ display: 'flex', alignItems: 'center', gap: 4, fontSize: 11, color: 'var(--text-secondary)' }}>
      {icon}
      {value}
    </div>
  );
}

function btnStyle(bg: string, disabled: boolean) {
  return {
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'center',
    gap: 6,
    padding: '8px 14px',
    background: disabled ? 'var(--bg-card)' : bg,
    border: `1px solid ${disabled ? 'var(--border)' : bg}`,
    borderRadius: 5,
    color: disabled ? 'var(--text-muted)' : '#fff',
    fontSize: 12,
    fontWeight: 600,
    cursor: disabled ? 'not-allowed' : 'pointer',
    transition: 'all 0.15s',
  };
}

function fmt(n: number): string {
  if (n > 999999) return `${(n / 1000000).toFixed(1)}M`;
  if (n > 999) return `${(n / 1000).toFixed(1)}k`;
  return String(n);
}
