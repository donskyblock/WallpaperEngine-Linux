import { useState } from 'react';
import { Play, Globe, Box, Star } from 'lucide-react';
import type { WorkshopItem } from '../../types';

const TYPE_ICONS: Record<string, React.ReactNode> = {
  video: <Play size={10} />,
  web: <Globe size={10} />,
  scene: <Box size={10} />,
};

const TYPE_COLORS: Record<string, string> = {
  video: '#4d9eff',
  web: '#4caf50',
  scene: '#a855f7',
};

interface Props {
  item: WorkshopItem;
  selected: boolean;
  onClick: () => void;
}

export default function WallpaperCard({ item, selected, onClick }: Props) {
  const [imgLoaded, setImgLoaded] = useState(false);
  const [imgError, setImgError] = useState(false);
  const type = item.content_type.toLowerCase();
  const typeColor = TYPE_COLORS[type] ?? '#888';
  const typeIcon = TYPE_ICONS[type];

  const score = Math.round(item.score * 100);
  const subs = item.subscriptions > 999
    ? `${(item.subscriptions / 1000).toFixed(1)}k`
    : String(item.subscriptions);

  return (
    <div
      onClick={onClick}
      style={{
        position: 'relative',
        aspectRatio: '16/9',
        borderRadius: 6,
        overflow: 'hidden',
        cursor: 'pointer',
        border: selected ? `2px solid var(--accent)` : '2px solid transparent',
        background: 'var(--bg-card)',
        transition: 'border-color 0.15s, transform 0.15s',
      }}
      onMouseEnter={(e) => {
        (e.currentTarget as HTMLDivElement).style.transform = 'scale(1.02)';
      }}
      onMouseLeave={(e) => {
        (e.currentTarget as HTMLDivElement).style.transform = 'scale(1)';
      }}
    >
      {!imgLoaded && !imgError && (
        <div
          style={{
            position: 'absolute',
            inset: 0,
            background: 'linear-gradient(135deg, #1a1a2e, #16213e)',
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'center',
          }}
        >
          <div style={{
            width: 20,
            height: 20,
            border: '2px solid var(--border)',
            borderTopColor: 'var(--accent)',
            borderRadius: '50%',
            animation: 'spin 0.8s linear infinite',
          }} />
        </div>
      )}

      {imgError && (
        <div style={{
          position: 'absolute',
          inset: 0,
          background: '#1a1a2e',
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
          color: 'var(--text-muted)',
          fontSize: 11,
        }}>
          No preview
        </div>
      )}

      {!imgError && (
        <img
          src={item.preview_url}
          alt={item.title}
          loading="lazy"
          onLoad={() => setImgLoaded(true)}
          onError={() => setImgError(true)}
          style={{
            width: '100%',
            height: '100%',
            objectFit: 'cover',
            opacity: imgLoaded ? 1 : 0,
            transition: 'opacity 0.2s',
          }}
        />
      )}

      <div
        style={{
          position: 'absolute',
          inset: 0,
          background: 'linear-gradient(to top, rgba(0,0,0,0.85) 0%, rgba(0,0,0,0.3) 50%, transparent 100%)',
          opacity: 0,
          transition: 'opacity 0.15s',
        }}
        className="card-overlay"
      />

      <div
        style={{
          position: 'absolute',
          bottom: 0,
          left: 0,
          right: 0,
          padding: '8px 8px 6px',
          background: 'linear-gradient(to top, rgba(0,0,0,0.9), transparent)',
        }}
      >
        <div style={{
          fontSize: 11,
          fontWeight: 500,
          color: '#fff',
          overflow: 'hidden',
          textOverflow: 'ellipsis',
          whiteSpace: 'nowrap',
          marginBottom: 3,
        }}>
          {item.title}
        </div>
        <div style={{ display: 'flex', alignItems: 'center', gap: 6 }}>
          <span style={{
            display: 'flex',
            alignItems: 'center',
            gap: 3,
            fontSize: 10,
            color: typeColor,
            background: `${typeColor}22`,
            padding: '1px 5px',
            borderRadius: 3,
          }}>
            {typeIcon}
            {item.content_type}
          </span>
          {score > 0 && (
            <span style={{ display: 'flex', alignItems: 'center', gap: 2, fontSize: 10, color: '#f0c040' }}>
              <Star size={9} fill="#f0c040" />
              {score}%
            </span>
          )}
          <span style={{ fontSize: 10, color: 'var(--text-muted)', marginLeft: 'auto' }}>
            {subs}
          </span>
        </div>
      </div>

      <style>{`@keyframes spin { to { transform: rotate(360deg); } }`}</style>
    </div>
  );
}
