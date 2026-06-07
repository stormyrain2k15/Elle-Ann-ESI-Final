package com.elleann.android.ui.world

import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.elleann.android.data.AppContainerExtended
import com.elleann.android.navigation.ElleRoutes
import com.elleann.android.ui.components.*
import com.elleann.android.ui.theme.*

data class WorldSection(val label: String, val icon: ImageVector, val route: String, val subtitle: String)

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun WorldScreen(
    containerExtended: AppContainerExtended,
    onNavigate: (String) -> Unit,
    onSettings: () -> Unit,
) {
    val sections = listOf(
        WorldSection("Goals",           Icons.Rounded.Flag,          ElleRoutes.WORLD_GOALS,        "Active drives and progress"),
        WorldSection("Thoughts",        Icons.Rounded.Psychology,    ElleRoutes.WORLD_THOUGHTS,     "Elle's autonomous thought log"),
        WorldSection("Private Inner Life", Icons.Rounded.Lock,       ElleRoutes.WORLD_INNER_LIFE,   "Unguarded internal monologue"),
        WorldSection("Autobiography",   Icons.Rounded.MenuBook,      ElleRoutes.WORLD_AUTOBIOGRAPHY,"Self-written life story"),
        WorldSection("Identity",        Icons.Rounded.Person,        ElleRoutes.WORLD_IDENTITY,     "Traits, snapshots, growth log"),
        WorldSection("Felt Time",       Icons.Rounded.Schedule,      ElleRoutes.WORLD_FELT_TIME,    "Subjective experience of time"),
        WorldSection("Consent Log",     Icons.Rounded.VerifiedUser,  ElleRoutes.WORLD_CONSENT,      "What Elle agreed or refused"),
        WorldSection("Observatory",     Icons.Rounded.Radar,         ElleRoutes.WORLD_OBSERVATORY,  "All 102 emotion dimensions live"),
        WorldSection("Patterns",        Icons.Rounded.Timeline,      ElleRoutes.WORLD_PATTERNS,     "Recurring emotional threads"),
        WorldSection("Learning",        Icons.Rounded.School,        ElleRoutes.WORLD_LEARNING,     "Subjects, skills, milestones"),
        WorldSection("Capabilities",    Icons.Rounded.Build,         ElleRoutes.WORLD_CAPABILITIES, "What Elle can do"),
        WorldSection("Ethics",          Icons.Rounded.Balance,       ElleRoutes.WORLD_ETHICS,       "Moral framework"),
        WorldSection("Autonomy",        Icons.Rounded.AutoMode,      ElleRoutes.WORLD_AUTONOMY,     "Trust level, self-prompting"),
        WorldSection("Self-Image",      Icons.Rounded.Face,          ElleRoutes.WORLD_SELF_IMAGE,   "How Elle sees herself"),
        WorldSection("X-Chronicle",     Icons.Rounded.Favorite,      ElleRoutes.WORLD_X_CHRONICLE,  "Biological cycle and lifecycle"),
    )

    Scaffold(
        containerColor = IsyaNight,
        topBar = {
            TopAppBar(
                title = { Text("Elle's World", color = IsyaGold) },
                actions = {
                    IconButton(onClick = onSettings) { Icon(Icons.Rounded.Settings, "Settings", tint = IsyaMuted) }
                },
                colors = TopAppBarDefaults.topAppBarColors(containerColor = IsyaNight),
            )
        }
    ) { padding ->
        LazyColumn(
            modifier = Modifier.fillMaxSize().padding(padding),
            contentPadding = PaddingValues(16.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp),
        ) {
            items(sections) { sec ->
                Surface(
                    modifier = Modifier.fillMaxWidth().clickable { onNavigate(sec.route) },
                    shape = RoundedCornerShape(12.dp),
                    color = IsyaDusk,
                ) {
                    Row(
                        modifier = Modifier.padding(16.dp),
                        verticalAlignment = Alignment.CenterVertically,
                    ) {
                        Icon(sec.icon, null, tint = IsyaMagic, modifier = Modifier.size(28.dp))
                        Spacer(Modifier.width(14.dp))
                        Column(modifier = Modifier.weight(1f)) {
                            Text(sec.label, style = MaterialTheme.typography.titleSmall, color = IsyaCream, fontWeight = FontWeight.Medium)
                            Text(sec.subtitle, style = MaterialTheme.typography.bodySmall, color = IsyaMuted)
                        }
                        Icon(Icons.Rounded.ChevronRight, null, tint = IsyaMuted)
                    }
                }
            }
        }
    }
}
