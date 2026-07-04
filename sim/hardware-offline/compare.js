/** Output comparison with module-kind tolerance policy. */

export const TOLERANCE_PASSTHROUGH = { maxAbsDeviation: 0, bitExact: true };
export const TOLERANCE_DELAY = { maxAbsDeviation: 2, bitExact: false };

export function toleranceForModuleKind(moduleKind) {
  switch (moduleKind) {
    case 'passthrough':
      return TOLERANCE_PASSTHROUGH;
    case 'delay':
      return TOLERANCE_DELAY;
    default:
      throw new Error(`unsupported module kind for comparison: ${moduleKind}`);
  }
}

export function maxAbsDiff(a, b) {
  const n = Math.min(a.length, b.length);
  let max = 0;
  for (let i = 0; i < n; i++) {
    const diff = Math.abs(a[i] - b[i]);
    if (diff > max) {
      max = diff;
    }
  }
  return max;
}

export function compareCaptureToReference({
  moduleKind,
  capture,
  reference,
  compareFromPeriod = 0,
  tolerance = toleranceForModuleKind(moduleKind),
}) {
  const failures = [];
  const periodCount = Math.min(capture.length, reference.length);
  let comparedPeriods = 0;

  for (let period = compareFromPeriod; period < periodCount; period++) {
    comparedPeriods++;
    for (const path of ['downstreamOut', 'upstreamOut']) {
      const observed = capture[period][path];
      const expected = reference[period][path];
      const deviation = maxAbsDiff(observed, expected);
      if (tolerance.bitExact ? deviation !== 0 : deviation > tolerance.maxAbsDeviation) {
        failures.push({
          period,
          path,
          maxDeviation: deviation,
          threshold: tolerance.bitExact ? 0 : tolerance.maxAbsDeviation,
        });
      }
    }
  }

  return {
    pass: failures.length === 0,
    moduleKind,
    comparedPeriods,
    compareFromPeriod,
    tolerance,
    failures,
    firstFailure: failures[0] ?? null,
  };
}

export function formatComparisonReport(result) {
  if (result.pass) {
    return `PASS ${result.moduleKind}: ${result.comparedPeriods} periods from index ${result.compareFromPeriod}`;
  }
  const first = result.firstFailure;
  return [
    `FAIL ${result.moduleKind}`,
    `  first offending period=${first.period} path=${first.path}`,
    `  max deviation=${first.maxDeviation} threshold=${first.threshold}`,
    `  compared periods=${result.comparedPeriods} from index ${result.compareFromPeriod}`,
  ].join('\n');
}
