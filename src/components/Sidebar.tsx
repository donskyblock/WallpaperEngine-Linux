import { Monitor, Settings, BookOpen, Compass } from 'lucide-react';
import { useAppStore } from '../store/app-store';
import type { NavView } from '../types';

interface NavItem {
  id: NavView;
  label: string;
  icon: React.ReactNode;
}

const NAV: NavItem[] = [
  { id: 'workshop', label: 'Workshop', icon: <Compass size={16} /> },
  { id: 'library', label: 'My Library', icon: <BookOpen size={16} /> },
];

const BOTTOM_NAV: NavItem[] = [
  { id: 'settings', label: 'Settings', icon: <Settings size={16} /> },
];

const s = {
  sidebar: {
    width: 200,
    minWidth: 200,
    background: 'var(--bg-sidebar)',
    borderRight: '1px solid var(--border)',
    display: 'flex',
    flexDirection: 'column' as const,
    padding: '0',
    overflow: 'hidden',
  },
  logo: {
    padding: '20px 16px 16px',
    borderBottom: '1px solid var(--border)',
    display: 'flex',
    alignItems: 'center',
    gap: 10,
  },
  logoIcon: {
    width: 28,
    height: 28,
    background: 'linear-gradient(135deg, #4d9eff, #a855f7)',
    borderRadius: 6,
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'center',
    flexShrink: 0,
  },
  logoText: {
    fontWeight: 700,
    fontSize: 13,
    color: 'var(--text-primary)',
    lineHeight: 1.2,
  },
  logoSub: {
    fontSize: 10,
    color: 'var(--text-muted)',
  },
  section: {
    padding: '12px 8px 4px',
  },
  sectionLabel: {
    fontSize: 10,
    fontWeight: 600,
    color: 'var(--text-muted)',
    textTransform: 'uppercase' as const,
    letterSpacing: '0.08em',
    padding: '0 8px',
    marginBottom: 4,
  },
  navItem: (active: boolean) => ({
    display: 'flex',
    alignItems: 'center',
    gap: 8,
    padding: '7px 8px',
    borderRadius: 6,
    cursor: 'pointer',
    color: active ? 'var(--accent)' : 'var(--text-secondary)',
    background: active ? 'rgba(77, 158, 255, 0.1)' : 'transparent',
    fontWeight: active ? 600 : 400,
    transition: 'all 0.15s',
  }),
  spacer: { flex: 1 },
  bottomSection: {
    padding: '8px',
    borderTop: '1px solid var(--border)',
  },
};

export default function Sidebar() {
  const { view, setView } = useAppStore();

  return (
    <div style={s.sidebar}>
      <div style={s.logo}>
        <div style={s.logoIcon}>
          <Monitor size={14} color="#fff" />
        </div>
        <div>
          <div style={s.logoText}>Wallpaper Engine</div>
          <div style={s.logoSub}>Linux Edition</div>
        </div>
      </div>

      <div style={s.section}>
        <div style={s.sectionLabel}>Browse</div>
        {NAV.map((item) => (
          <NavButton
            key={item.id}
            item={item}
            active={view === item.id}
            onClick={() => setView(item.id)}
          />
        ))}
      </div>

      <div style={s.spacer} />

      <div style={s.bottomSection}>
        {BOTTOM_NAV.map((item) => (
          <NavButton
            key={item.id}
            item={item}
            active={view === item.id}
            onClick={() => setView(item.id)}
          />
        ))}
      </div>
    </div>
  );
}

function NavButton({
  item,
  active,
  onClick,
}: {
  item: NavItem;
  active: boolean;
  onClick: () => void;
}) {
  return (
    <div
      style={s.navItem(active)}
      onClick={onClick}
      onMouseEnter={(e) => {
        if (!active) {
          (e.currentTarget as HTMLDivElement).style.background = 'var(--bg-card-hover)';
          (e.currentTarget as HTMLDivElement).style.color = 'var(--text-primary)';
        }
      }}
      onMouseLeave={(e) => {
        if (!active) {
          (e.currentTarget as HTMLDivElement).style.background = 'transparent';
          (e.currentTarget as HTMLDivElement).style.color = 'var(--text-secondary)';
        }
      }}
    >
      {item.icon}
      <span>{item.label}</span>
    </div>
  );
}
