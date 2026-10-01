import { useEffect } from 'react';
import { Play, RefreshCw, FolderOpen, Film, Image } from 'lucide-react';
import { useAppStore } from '../../store/app-store';

export default function Library() {
  const { libraryItems, isLoadingLibrary, loadLibrary, applyLibraryItem } = useAppStore();

  useEffect(() => {
    loadLibrary();
  }, []);

  return (
    <div style={{ display: 'flex', flexDirection: 'column', height: '100%', overflow: 'hidden' }}>
      <div style={{
        padding: '10px 14px',
        borderBottom: '1px solid var(--border)',
        display: 'flex',
        alignItems: 'center',
        gap: 10,
        background: 'var(--bg-surface)',
        flexShrink: 0,
      }}>
        <span style={{ fontSize: 13, fontWeight: 600, color: 'var(--text-primary)' }}>My Library</span>
        <span style={{ fontSize: 11, color: 'var(--text-muted)' }}>{libraryItems.length} items</span>
        <div style={{ flex: 1 }} />
        <button
          onClick={loadLibrary}
          disabled={isLoadingLibrary}
          style={{
            display: 'flex',
            alignItems: 'center',
            gap: 5,
            padding: '5px 10px',
            background: 'var(--bg-card)',
            border: '1px solid var(--border)',
            borderRadius: 4,
            color: 'var(--text-secondary)',
            fontSize: 12,
            cursor: isLoadingLibrary ? 'not-allowed' : 'pointer',
          }}
        >
          <RefreshCw size={12} style={{ animation: isLoadingLibrary ? 'spin 0.8s linear infinite' : 'none' }} />
          Refresh
        </button>
      </div>

      <div style={{ flex: 1, overflowY: 'auto', padding: 12 }}>
        {libraryItems.length === 0 && !isLoadingLibrary && (
          <EmptyState />
        )}

        {libraryItems.length > 0 && (
          <div style={{
            display: 'grid',
            gridTemplateColumns: 'repeat(auto-fill, minmax(200px, 1fr))',
            gap: 8,
          }}>
            {libraryItems.map((item) => (
              <LibraryCard
                key={item.id}
                item={item}
                onApply={() => applyLibraryItem(item)}
              />
            ))}
          </div>
        )}
      </div>
      <style>{`@keyframes spin { to { transform: rotate(360deg); } }`}</style>
    </div>
  );
}

function LibraryCard({ item, onApply }: { item: import('../../types').LibraryItem; onApply: () => void }) {
  const type = item.content_type.toLowerCase();
  const isVideo = type === 'video';

  return (
    <div style={{
      position: 'relative',
      aspectRatio: '16/9',
      borderRadius: 6,
      overflow: 'hidden',
      background: 'var(--bg-card)',
      border: '2px solid transparent',
      cursor: 'pointer',
      transition: 'border-color 0.15s',
    }}
    onMouseEnter={(e) => {
      (e.currentTarget as HTMLDivElement).style.borderColor = 'var(--accent)';
      const overlay = e.currentTarget.querySelector('.lib-overlay') as HTMLElement;
      if (overlay) overlay.style.opacity = '1';
    }}
    onMouseLeave={(e) => {
      (e.currentTarget as HTMLDivElement).style.borderColor = 'transparent';
      const overlay = e.currentTarget.querySelector('.lib-overlay') as HTMLElement;
      if (overlay) overlay.style.opacity = '0';
    }}
    >
      {item.preview_path ? (
        <img
          src={`file://${item.preview_path}`}
          alt={item.title}
          style={{ width: '100%', height: '100%', objectFit: 'cover' }}
          onError={(e) => { (e.currentTarget as HTMLImageElement).style.display = 'none'; }}
        />
      ) : (
        <div style={{
          width: '100%',
          height: '100%',
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
          background: 'linear-gradient(135deg, #1a1a2e, #16213e)',
        }}>
          {isVideo ? <Film size={28} color="var(--text-muted)" /> : <Image size={28} color="var(--text-muted)" />}
        </div>
      )}

      <div className="lib-overlay" style={{
        position: 'absolute',
        inset: 0,
        background: 'rgba(0,0,0,0.6)',
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'center',
        opacity: 0,
        transition: 'opacity 0.15s',
      }}>
        <button
          onClick={onApply}
          style={{
            display: 'flex',
            alignItems: 'center',
            gap: 6,
            padding: '8px 16px',
            background: 'var(--accent)',
            border: 'none',
            borderRadius: 5,
            color: '#fff',
            fontSize: 12,
            fontWeight: 600,
            cursor: 'pointer',
          }}
        >
          <Play size={12} />
          Apply
        </button>
      </div>

      <div style={{
        position: 'absolute',
        bottom: 0,
        left: 0,
        right: 0,
        padding: '6px 8px',
        background: 'linear-gradient(to top, rgba(0,0,0,0.9), transparent)',
      }}>
        <div style={{ fontSize: 11, fontWeight: 500, color: '#fff', overflow: 'hidden', textOverflow: 'ellipsis', whiteSpace: 'nowrap' }}>
          {item.title}
        </div>
      </div>
    </div>
  );
}

function EmptyState() {
  const { settings, setView } = useAppStore();
  return (
    <div style={{
      display: 'flex',
      flexDirection: 'column',
      alignItems: 'center',
      justifyContent: 'center',
      height: '60vh',
      gap: 12,
      color: 'var(--text-secondary)',
    }}>
      <FolderOpen size={48} color="var(--text-muted)" />
      <div style={{ fontSize: 14, fontWeight: 600 }}>Library is empty</div>
      <div style={{ fontSize: 12, color: 'var(--text-muted)', textAlign: 'center', maxWidth: 280 }}>
        Download wallpapers from the Workshop, or add video/image files to:
        <br />
        <code style={{ fontSize: 10, color: 'var(--accent)', wordBreak: 'break-all' }}>
          {settings.download_dir || '~/.local/share/wpe-linux/wallpapers'}
        </code>
      </div>
      <button
        onClick={() => setView('workshop')}
        style={{
          padding: '7px 16px',
          background: 'var(--accent)',
          border: 'none',
          borderRadius: 5,
          color: '#fff',
          fontSize: 12,
          fontWeight: 600,
          cursor: 'pointer',
        }}
      >
        Browse Workshop
      </button>
    </div>
  );
}
