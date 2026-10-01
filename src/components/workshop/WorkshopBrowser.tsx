import { useEffect, useRef, useCallback } from 'react';
import { Search, Loader, AlertCircle, ChevronDown } from 'lucide-react';
import { useAppStore } from '../../store/app-store';
import WallpaperCard from './WallpaperCard';
import WallpaperDetail from './WallpaperDetail';
import type { SortType, TypeFilter } from '../../types';

const SORT_OPTIONS: { value: SortType; label: string }[] = [
  { value: 'vote', label: 'Top Rated' },
  { value: 'new', label: 'Newest' },
  { value: 'trend', label: 'Trending' },
  { value: 'subscriptions', label: 'Most Subscribed' },
];

const TYPE_OPTIONS: { value: TypeFilter; label: string }[] = [
  { value: 'all', label: 'All Types' },
  { value: 'Video', label: 'Video' },
  { value: 'Scene', label: 'Scene' },
  { value: 'Web', label: 'Web' },
];

export default function WorkshopBrowser() {
  const {
    workshopItems, workshopTotal, isLoadingWorkshop, workshopError,
    workshopSearch, workshopSort, workshopTypeFilter,
    selectedItem, setSelectedItem,
    searchWorkshop, loadNextPage, setSort, setTypeFilter,
    settings,
  } = useAppStore();

  const sentinelRef = useRef<HTMLDivElement>(null);
  const searchRef = useRef<HTMLInputElement>(null);
  const hasMore = workshopItems.length < workshopTotal;

  // Load on mount if API key set
  useEffect(() => {
    if (settings.steam_api_key && workshopItems.length === 0) {
      searchWorkshop('');
    }
  }, [settings.steam_api_key]);

  // Infinite scroll
  useEffect(() => {
    const el = sentinelRef.current;
    if (!el) return;
    const obs = new IntersectionObserver(
      (entries) => {
        if (entries[0].isIntersecting && hasMore && !isLoadingWorkshop) {
          loadNextPage();
        }
      },
      { rootMargin: '200px' }
    );
    obs.observe(el);
    return () => obs.disconnect();
  }, [hasMore, isLoadingWorkshop, loadNextPage]);

  const handleSearch = useCallback((e: React.FormEvent) => {
    e.preventDefault();
    searchWorkshop(searchRef.current?.value ?? '');
  }, [searchWorkshop]);

  const noApiKey = !settings.steam_api_key;

  return (
    <div style={{ display: 'flex', flex: 1, overflow: 'hidden' }}>
      <div style={{ flex: 1, display: 'flex', flexDirection: 'column', overflow: 'hidden' }}>

        {/* Top bar */}
        <div style={{
          padding: '10px 14px',
          borderBottom: '1px solid var(--border)',
          display: 'flex',
          alignItems: 'center',
          gap: 10,
          flexShrink: 0,
          background: 'var(--bg-surface)',
        }}>
          <form onSubmit={handleSearch} style={{ flex: 1, display: 'flex', gap: 8 }}>
            <div style={{ position: 'relative', flex: 1 }}>
              <Search size={13} style={{ position: 'absolute', left: 9, top: '50%', transform: 'translateY(-50%)', color: 'var(--text-muted)', pointerEvents: 'none' }} />
              <input
                ref={searchRef}
                defaultValue={workshopSearch}
                placeholder="Search wallpapers…"
                style={{
                  width: '100%',
                  padding: '6px 10px 6px 28px',
                  background: 'var(--bg-card)',
                  border: '1px solid var(--border)',
                  borderRadius: 5,
                  color: 'var(--text-primary)',
                  fontSize: 12,
                }}
              />
            </div>
            <button type="submit" style={{ padding: '6px 12px', background: 'var(--accent)', border: 'none', borderRadius: 5, color: '#fff', fontSize: 12, fontWeight: 600, cursor: 'pointer' }}>
              Search
            </button>
          </form>

          <Select
            value={workshopSort}
            options={SORT_OPTIONS}
            onChange={(v) => setSort(v as SortType)}
          />
          <Select
            value={workshopTypeFilter}
            options={TYPE_OPTIONS}
            onChange={(v) => setTypeFilter(v as TypeFilter)}
          />
        </div>

        {/* Content */}
        <div style={{ flex: 1, overflowY: 'auto', padding: 12 }}>
          {noApiKey && (
            <div style={{
              display: 'flex',
              flexDirection: 'column',
              alignItems: 'center',
              justifyContent: 'center',
              height: '60%',
              gap: 12,
              color: 'var(--text-secondary)',
            }}>
              <AlertCircle size={40} color="var(--text-muted)" />
              <div style={{ fontSize: 14, fontWeight: 600 }}>Steam API Key Required</div>
              <div style={{ fontSize: 12, color: 'var(--text-muted)', textAlign: 'center', maxWidth: 300 }}>
                Add your free Steam Web API key in Settings to browse the workshop.
                Get one at <strong style={{ color: 'var(--accent)' }}>steamcommunity.com/dev/apikey</strong>
              </div>
            </div>
          )}

          {workshopError && !noApiKey && (
            <div style={{
              margin: '20px auto',
              maxWidth: 400,
              padding: '12px 16px',
              background: 'rgba(244,67,54,0.1)',
              border: '1px solid rgba(244,67,54,0.3)',
              borderRadius: 6,
              color: 'var(--error)',
              fontSize: 12,
              display: 'flex',
              gap: 8,
              alignItems: 'flex-start',
            }}>
              <AlertCircle size={14} style={{ flexShrink: 0, marginTop: 1 }} />
              {workshopError}
            </div>
          )}

          {!noApiKey && !workshopError && workshopItems.length === 0 && !isLoadingWorkshop && (
            <div style={{ textAlign: 'center', marginTop: 60, color: 'var(--text-muted)' }}>
              No wallpapers found.
            </div>
          )}

          {workshopItems.length > 0 && (
            <>
              <div style={{ marginBottom: 8, fontSize: 11, color: 'var(--text-muted)' }}>
                {workshopTotal.toLocaleString()} wallpapers
              </div>
              <div style={{
                display: 'grid',
                gridTemplateColumns: 'repeat(auto-fill, minmax(200px, 1fr))',
                gap: 8,
              }}>
                {workshopItems.map((item) => (
                  <WallpaperCard
                    key={item.id}
                    item={item}
                    selected={selectedItem?.id === item.id}
                    onClick={() => setSelectedItem(selectedItem?.id === item.id ? null : item)}
                  />
                ))}
              </div>

              {hasMore && (
                <div ref={sentinelRef} style={{ height: 40, display: 'flex', alignItems: 'center', justifyContent: 'center' }}>
                  {isLoadingWorkshop && <Loader size={16} color="var(--text-muted)" style={{ animation: 'spin 0.8s linear infinite' }} />}
                </div>
              )}
            </>
          )}

          {workshopItems.length === 0 && isLoadingWorkshop && (
            <div style={{
              display: 'grid',
              gridTemplateColumns: 'repeat(auto-fill, minmax(200px, 1fr))',
              gap: 8,
            }}>
              {Array.from({ length: 18 }).map((_, i) => (
                <SkeletonCard key={i} />
              ))}
            </div>
          )}
        </div>
      </div>

      {selectedItem && <WallpaperDetail />}
    </div>
  );
}

function Select<T extends string>({
  value, options, onChange,
}: {
  value: T;
  options: { value: T; label: string }[];
  onChange: (v: T) => void;
}) {
  return (
    <div style={{ position: 'relative' }}>
      <select
        value={value}
        onChange={(e) => onChange(e.target.value as T)}
        style={{
          appearance: 'none',
          padding: '6px 24px 6px 10px',
          background: 'var(--bg-card)',
          border: '1px solid var(--border)',
          borderRadius: 5,
          color: 'var(--text-primary)',
          fontSize: 12,
          cursor: 'pointer',
        }}
      >
        {options.map((o) => (
          <option key={o.value} value={o.value}>{o.label}</option>
        ))}
      </select>
      <ChevronDown size={11} style={{ position: 'absolute', right: 7, top: '50%', transform: 'translateY(-50%)', pointerEvents: 'none', color: 'var(--text-muted)' }} />
    </div>
  );
}

function SkeletonCard() {
  return (
    <div style={{
      aspectRatio: '16/9',
      borderRadius: 6,
      background: 'linear-gradient(90deg, var(--bg-card) 25%, var(--bg-card-hover) 50%, var(--bg-card) 75%)',
      backgroundSize: '200% 100%',
      animation: 'shimmer 1.5s infinite',
    }}>
      <style>{`@keyframes shimmer { 0% { background-position: 200% 0; } 100% { background-position: -200% 0; } } @keyframes spin { to { transform: rotate(360deg); } }`}</style>
    </div>
  );
}
