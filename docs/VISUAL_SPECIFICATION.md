# CustomShellBar Visual Specification

**Status:** Design baseline for the first visual prototype  
**Target:** Windows 10, x64, per-monitor DPI aware  
**Reference asset:** `docs/assets/CustomShellBar-prototype-mockup.png`

> The mockup is concept artwork. Release screenshots must clearly distinguish implemented features from planned features.

## 1. Design intent

CustomShellBar should feel integrated with Windows while remaining visibly distinct: dark floating glass surfaces, precise warm-orange borders, generous icon spacing, and no opaque full-width taskbar strip. Explorer remains active in the background; these specifications apply only to the separate CustomShellBar windows.

## 2. Desktop composition

The default layout contains three independent surfaces:

1. **Application capsule** centered near the bottom edge.
2. **System capsule** aligned near the bottom-right edge.
3. **Start panel** anchored above the application capsule when open.

The transparent desktop gap between the application and system capsules must remain real click-through desktop space. Do not imitate separation by drawing both capsules on one full-width opaque window.

## 3. Design tokens

| Token | Default | Implementation note |
|---|---:|---|
| Accent | `#FF7A2F` | Warm orange border and selected control state |
| Accent hover | `#FF965C` | Lighter orange for hover/focus |
| Surface base | `#141414` | Apply opacity rather than using pure opaque black |
| Surface opacity | 82% | User-adjustable range: 55% to 95% |
| Border opacity | 90% | Keep borders visible on both bright and dark wallpaper |
| Primary text | `#FFFFFF` | Target at least 4.5:1 contrast for normal text |
| Secondary text | `#C8C8C8` | Avoid below 70% opacity on glass surfaces |
| Muted text | `#A0A0A0` | Metadata only, not critical actions |
| Divider | `#FFFFFF` at 14% | One physical pixel at 100% DPI |
| Shadow | `#000000` at 35% | Soft, restrained, outside surface bounds |
| Font | Segoe UI Variable, fallback Segoe UI | Use DirectWrite |

All pixel dimensions below are **device-independent pixels (DIPs)** and must scale per monitor.

## 4. Application capsule

- Height: **68 DIP**.
- Minimum width: **360 DIP**; width follows content.
- Maximum width: available work area minus **2 x 24 DIP**.
- Bottom offset: **12 DIP** from monitor work-area edge.
- Corner radius: **22 DIP**.
- Border: **1 DIP**, accent color at 90% opacity.
- Inner horizontal padding: **18 DIP**.
- Inner vertical padding: **10 DIP**.
- Icon button hit target: **44 x 44 DIP** minimum.
- Icon artwork: **24 x 24 DIP**; allow **28 DIP** for apps whose source icon has excessive internal padding.
- Gap between buttons: **6 DIP**.
- Active indicator: **4 x 4 DIP** circular dot, centered, 4 DIP above the inner bottom edge.
- Running but inactive indicator: accent at 65%; foreground app indicator: accent at 100%.
- Hover background: white at 8%; pressed background: white at 13%.
- Keyboard focus: 2 DIP inner focus stroke using accent hover.

The application capsule may grow as applications open, but it must not overlap the system capsule. When space is insufficient, reduce inter-button spacing to 3 DIP, then collapse grouped windows before enabling horizontal overflow.

## 5. System capsule

- Height: **60 DIP**.
- Right offset: **18 DIP**.
- Bottom offset: **12 DIP**.
- Corner radius: **20 DIP**.
- Border and surface tokens match the application capsule.
- Horizontal padding: **16 DIP**.
- System icon hit target: **36 x 36 DIP** minimum.
- Time block: right-aligned, **14 DIP** time and **12 DIP** date, 2 DIP line gap.
- Default order: overflow chevron, network, audio, battery, clock/date.
- Separation between status icons and clock: **12 DIP**.

System icons must be vector geometry or high-quality monochrome assets. Do not use glyphs that become illegible below 16 physical pixels.

## 6. Start panel

- Default width: **520 DIP**.
- Default height: **620 DIP**, constrained to available work area.
- Anchor: **8 DIP** above the application capsule; align to the capsule's left edge unless that would leave the monitor work area.
- Corner radius: **20 DIP**.
- Border: **1 DIP** accent at 85% opacity.
- Outer padding: **24 DIP**.
- Shadow blur target: **28 DIP**, 8 DIP downward offset.

### Header

- Product title: 22 DIP Semibold.
- Optional subtitle: 13 DIP, secondary text.
- Search box: 44 DIP high, 14 DIP radius, white 6% fill, white 20% border.

### Pinned apps

- Four columns at the 520 DIP default width.
- Tile: **96 x 96 DIP**.
- Tile radius: **12 DIP**.
- App icon: **36 x 36 DIP**.
- Label: 13 DIP, centered, one line with ellipsis.
- Grid gap: **12 DIP**.

### Quick controls

- Volume slider track: 4 DIP high; thumb: 18 DIP diameter.
- Quick-control buttons: minimum 44 DIP high, 12 DIP radius.
- Selected state: accent at 16% fill and 1 DIP accent border.

### Footer and safety action

- Profile and power controls remain visible at the bottom.
- The action **Exit and restore Windows taskbar** must be reachable without navigating to a secondary settings page.
- Safety action minimum hit target: **44 DIP** high.
- Destructive styling is not required; clarity and availability are required.

## 7. Motion

Respect the Windows **Show animations in Windows** accessibility setting.

| Interaction | Duration | Curve |
|---|---:|---|
| Capsule entrance | 180 ms | cubic ease-out |
| Start panel open | 200 ms | cubic ease-out |
| Start panel close | 140 ms | cubic ease-in |
| Hover transition | 90 ms | linear/ease-out |
| Reorder movement | 180 ms | ease-in-out |

Avoid spring overshoot in the recovery and power controls. When animations are disabled, update state immediately.

## 8. Blur, transparency, and fallback

Preferred renderer: Direct2D and DirectWrite, with DirectComposition for independent surfaces and animation. Blur is decorative, not structural.

Fallback order:

1. DWM/DirectComposition glass where available.
2. Translucent dark surface without blur.
3. Opaque `#202020` high-contrast surface.

Borders, text, focus, hit testing, and recovery controls must remain usable in every fallback mode.

## 9. Scaling and monitor behavior

- Declare **Per-Monitor V2 DPI awareness**.
- Recalculate all DIPs after `WM_DPICHANGED`.
- Test at 100%, 125%, 150%, 175%, and 200% scaling.
- Keep a minimum 8 DIP visual gap from monitor edges.
- Rebuild placement after resolution, rotation, monitor, primary-display, or work-area changes.
- Render one application/system capsule pair per enabled monitor unless settings specify primary-only mode.

## 10. Input and accessibility

- Every interactive target: at least **44 x 44 DIP**, except compact tray glyphs whose surrounding button still meets the target.
- Full keyboard navigation with visible focus.
- Escape closes the Start panel and flyouts, not the application.
- Provide accessible names, roles, toggle states, and values through Microsoft UI Automation.
- Do not encode running, selected, muted, connected, or warning states by color alone.
- Support Windows high-contrast mode with opaque system colors and visible borders.
- Tooltips appear after approximately 500 ms and never replace accessible names.

## 11. Interaction rules

- Start button opens or closes the Start panel.
- Clicking a running app activates it; clicking the foreground app minimizes it when that behavior is enabled.
- Right-clicking an app opens its jump/context menu without moving the capsule.
- Clicking empty desktop space between capsules must reach the desktop.
- Right-clicking an empty capsule area provides **Settings** and **Exit and restore Windows taskbar**.
- The emergency shortcut `Ctrl + Alt + Shift + F12` remains independent of visual focus.

## 12. Prototype acceptance checklist

- [ ] No visible full-width bar behind the floating capsules.
- [ ] Orange border remains continuous at all four rounded corners.
- [ ] App and system capsules are separate windows or independently hit-tested surfaces.
- [ ] Start panel remains inside the active monitor work area.
- [ ] All primary controls meet the 44 DIP target.
- [ ] Layout remains crisp at 100%, 125%, 150%, and 200% DPI.
- [ ] Active app state is visible without relying only on color.
- [ ] High-contrast fallback is usable.
- [ ] Exit-and-restore action is visible and keyboard reachable.
- [ ] Mockup-only features are labelled as planned until implemented.

## 13. First implementation scope

The first coded visual prototype should implement only:

- transparent AppBar host;
- application capsule with placeholder icon buttons;
- separate system capsule with placeholder status glyphs and a real clock;
- Start panel shell with non-functional pinned tiles;
- visible exit-and-restore command;
- DPI-aware borders, rounded corners, hover, focus, and fallback rendering.

Real task enumeration, app launching, audio, Wi-Fi, battery, third-party tray icons, and search remain separate milestones after watchdog recovery is proven.
