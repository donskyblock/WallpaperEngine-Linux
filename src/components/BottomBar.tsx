import { Monitor, ChevronDown, Square } from 'lucide-react';
import { useState } from 'react';
import { useAppStore } from '../store/app-store';

export default function BottomBar() {
  const { displays, selectedDisplay, setSelectedDisplay, stopWallpaper, isSettingWallpaper } = useAppStore();
  const [open, setOpen] = useState(false);

  const current = displays.find((d) => d.name === selectedDisplay);
  const label = current
    ? `${current.name}  ${current.width}×${current.height}@${Math.round(current.refresh_rate)}Hz`
    : selectedDisplay || 'No display';

  return (
    <div
      style={{
        height: 48,
        background: 'var(--bg-sidebar)',
        borderTop: '1px solid var(--border)',
        display: 'flex',
        alignItems: 'center',
        padding: '0 16px',
        gap: 12,
        flexShrink: 0,
        position: 'relative',
      }}
    >
      <Monitor size={14} color="var(--text-muted)" />
      <span style={{ color: 'var(--text-secondary)', fontSize: 12 }}>Display:</span>

      <div style={{ position: 'relative' }}>
        <button
          onClick={() => setOpen((o) => !o)}
          style={{
            display: 'flex',
            alignItems: 'center',
            gap: 6,
            padding: '4px 10px',
            background: 'var(--bg-card)',
            border: '1px solid var(--border)',
            borderRadius: 4,
            color: 'var(--text-primary)',
            fontSize: 12,
            cursor: 'pointer',
            minWidth: 220,
          }}
        >
          <span style={{ flex: 1, textAlign: 'left' }}>{label}</span>
          <ChevronDown size={12} />
        </button>

        {open && displays.length > 0 && (
          <div
            style={{
              position: 'absolute',
              bottom: '100%',
              left: 0,
              background: 'var(--bg-surface)',
              border: '1px solid var(--border)',
              borderRadius: 6,
              overflow: 'hidden',
              minWidth: 240,
              boxShadow: '0 -4px 16px rgba(0,0,0,0.4)',
              zIndex: 100,
            }}
          >
            {displays.map((d) => (
              <div
                key={d.name}
                onClick={() => { setSelectedDisplay(d.name); setOpen(false); }}
                style={{
                  padding: '8px 12px',
                  cursor: 'pointer',
                  color: d.name === selectedDisplay ? 'var(--accent)' : 'var(--text-primary)',
                  background: d.name === selectedDisplay ? 'rgba(77,158,255,0.08)' : 'transparent',
                  fontSize: 12,
                }}
                onMouseEnter={(e) => {
                  if (d.name !== selectedDisplay)
                    (e.currentTarget as HTMLDivElement).style.background = 'var(--bg-card-hover)';
                }}
                onMouseLeave={(e) => {
                  if (d.name !== selectedDisplay)
                    (e.currentTarget as HTMLDivElement).style.background = 'transparent';
                }}
              >
                <div style={{ fontWeight: 500 }}>{d.name}</div>
                <div style={{ color: 'var(--text-muted)', fontSize: 11 }}>
                  {d.width}×{d.height} @ {Math.round(d.refresh_rate)}Hz
                </div>
              </div>
            ))}
          </div>
        )}
      </div>

      <div style={{ flex: 1 }} />

      <button
        onClick={stopWallpaper}
        title="Stop wallpaper on selected display"
        style={{
          display: 'flex',
          alignItems: 'center',
          gap: 6,
          padding: '4px 10px',
          background: 'var(--bg-card)',
          border: '1px solid var(--border)',
          borderRadius: 4,
          color: 'var(--text-secondary)',
          fontSize: 12,
          cursor: 'pointer',
        }}
      >
        <Square size={11} />
        Stop
      </button>

      {isSettingWallpaper && (
        <span style={{ color: 'var(--accent)', fontSize: 12 }}>Applying…</span>
      )}
    </div>
  );
}
